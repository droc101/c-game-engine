//
// Created by droc101 on 10/7/2024.
//

#include "Slider.h"
#include <math.h>
#include <stdio.h>
#include "../../../Helpers/CommonAssets.h"
#include "../../../Helpers/Core/AssetReader.h"
#include "../../../Helpers/Core/Error.h"
#include "../../../Helpers/Core/Input.h"
#include "../../../Helpers/Core/MathEx.h"
#include "../../../Helpers/Graphics/Drawing.h"
#include "../../../Helpers/Graphics/Font.h"
#include "../../Vector2.h"

char *DefaultSliderLabelCallback(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->ControlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %.2f", data->label, data->value);
	return buf;
}

char *SliderLabelPercent(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->ControlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %.0f%%", data->label, data->value * 100);
	return buf;
}

char *SliderLabelInteger(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->ControlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %.0f", data->label, data->value);
	return buf;
}

Control *CreateSliderControl(const Vector2 position,
							 const Vector2 size,
							 char *label,
							 const SliderCallback callback,
							 const ControlAnchor anchor,
							 const double min,
							 const double max,
							 const double value,
							 const double step,
							 const double altStep,
							 SliderLabelFunction getLabel)
{
	if (getLabel == NULL)
	{
		getLabel = DefaultSliderLabelCallback;
	}

	Control *slider = CreateEmptyControl();
	slider->type = SLIDER;
	slider->position = position;
	slider->size = size;
	slider->anchor = anchor;

	slider->ControlData = malloc(sizeof(SliderData));
	CheckAlloc(slider->ControlData);
	SliderData *data = slider->ControlData;
	data->label = label;
	data->callback = callback;
	data->min = min;
	data->max = max;
	data->value = value;
	data->step = step;
	data->altStep = altStep;
	data->getLabel = getLabel;

	data->value = clamp(data->value, data->min, data->max);

	return slider;
}

void DestroySlider(const Control *c)
{
	SliderData *data = c->ControlData;
	free(data);
}

// ReSharper disable twice CppParameterMayBeConstPtrOrRef
void UpdateSlider(UiStack *stack, Control *c, Vector2 /*localMousePos*/, const uint ctlIndex)
{
	SliderData *data = c->ControlData;

	// handle l and r arrow keys
	if (stack->focusedControl == ctlIndex)
	{
		if (IsKeyJustPressed(SDL_SCANCODE_LEFT) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_LEFT))
		{
			ConsumeKey(SDL_SCANCODE_LEFT);
			ConsumeButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
			data->value -= data->step;
			if (data->value < data->min)
			{
				data->value = data->min;
			}
			if (data->callback != NULL)
			{
				data->callback(data->value);
			}
		} else if (IsKeyJustPressed(SDL_SCANCODE_RIGHT) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
		{
			ConsumeKey(SDL_SCANCODE_RIGHT);
			ConsumeButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
			data->value += data->step;
			if (data->value > data->max)
			{
				data->value = data->max;
			}
			if (data->callback != NULL)
			{
				data->callback(data->value);
			}
		}
	}

	// if (!IsMouseInRect(c->anchoredPosition, c->size))
	// {
	//     return;
	// }

	if (stack->activeControl != ctlIndex)
	{
		return;
	}

	const bool pressed = IsMouseButtonPressed(SDL_BUTTON_LEFT);

	if (pressed)
	{
		const double newVal = remap(GetMousePos().x - c->anchoredPosition.x, 0.0, c->size.x, data->min, data->max);
		data->value = newVal;

		// snap to step
		double step = data->step;
		if (IsKeyPressed(SDL_SCANCODE_LSHIFT) || IsKeyPressed(SDL_SCANCODE_RSHIFT))
		{
			step = data->altStep;
		}

		data->value = round(data->value / step) * step;

		if (data->value < data->min)
		{
			data->value = data->min;
		} else if (data->value > data->max)
		{
			data->value = data->max;
		}

		if (data->callback != NULL)
		{
			data->callback(data->value);
		}
	}

	data->value = clamp(data->value, data->min, data->max);
}

void DrawSlider(const Control *c, const ControlState /*state*/, const Vector2 position)
{
	DrawNinePatchTexture(c->anchoredPosition, c->size, 8, 8, TEXTURE("interface_slider"));

	const SliderData *data = (SliderData *)c->ControlData;
	const float handlePos = remap(data->value, data->min, data->max, 0, c->size.x - 18);

	DrawTexture(v2(position.x + handlePos + 4, position.y + 1),
				v2(10, c->size.y - 2),
				TEXTURE("interface_slider_thumb"));

	char *buf = data->getLabel(c);
	DrawTextAligned(buf,
					16,
					COLOR_BLACK,
					Vector2Add(position, v2s(2)),
					c->size,
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					smallFont);
	DrawTextAligned(buf, 16, COLOR_WHITE, position, c->size, FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, smallFont);
	free(buf);
}
