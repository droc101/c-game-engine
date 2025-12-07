//
// Created by droc101 on 10/7/2024.
//

#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/Slider.h>
#include <engine/uiStack/UiStack.h>
#include <math.h>
#include <SDL_gamecontroller.h>
#include <SDL_mouse.h>
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

char *DefaultSliderLabelCallback(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %.2f", data->label, data->value);
	return buf;
}

char *SliderLabelPercent(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %.0f%%", data->label, data->value * 100);
	return buf;
}

char *SliderLabelInteger(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->controlData;
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

	slider->controlData = malloc(sizeof(SliderData));
	CheckAlloc(slider->controlData);
	SliderData *data = slider->controlData;
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
	SliderData *data = c->controlData;
	free(data);
}

// ReSharper disable twice CppParameterMayBeConstPtrOrRef
void UpdateSlider(UiStack *stack, Control *c, Vector2 /*localMousePos*/, const uint32_t ctlIndex)
{
	SliderData *data = c->controlData;

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
				data->callback((float)data->value);
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
				data->callback((float)data->value);
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
			data->callback((float)data->value);
		}
	}

	data->value = clamp(data->value, data->min, data->max);
}

void DrawSlider(const Control *c, const ControlState /*state*/, const Vector2 position)
{
	DrawNinePatchTexture(c->anchoredPosition, c->size, 8, 8, TEXTURE("interface/slider"));

	const SliderData *data = (SliderData *)c->controlData;
	const float handlePos = remap(data->value, data->min, data->max, 0, c->size.x - 18);

	DrawTexture(v2(position.x + handlePos + 4, position.y + 1),
				v2(10, c->size.y - 2),
				TEXTURE("interface/slider_thumb"));

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
