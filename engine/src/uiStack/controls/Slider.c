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
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

float GetSliderValueAsFloat(const SliderData *data)
{
	float currentValue = 0.0f;
	switch (data->value.type)
	{
		case CONTROL_VALUE_BOOL:
			currentValue = *data->value.boolValue ? 1.0f : 0.0f;
			break;
		case CONTROL_VALUE_BYTE:
			currentValue = *data->value.byteValue;
			break;
		case CONTROL_VALUE_WORD:
			currentValue = *data->value.wordValue;
			break;
		case CONTROL_VALUE_DWORD:
			currentValue = *data->value.dwordValue;
			break;
		case CONTROL_VALUE_FLOAT:
			currentValue = *data->value.floatValue;
			break;
	}
	return currentValue;
}

static char *DefaultSliderLabelCallback(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %.2f", data->label, GetSliderValueAsFloat(data));
	return buf;
}

char *SliderLabelPercent(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %.0f%%", data->label, GetSliderValueAsFloat(data) * 100);
	return buf;
}

char *SliderLabelInteger(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %.0f", data->label, GetSliderValueAsFloat(data));
	return buf;
}

static void ClampSliderValue(SliderData *data)
{
	switch (data->value.type)
	{
		case CONTROL_VALUE_BOOL:
			*data->value.boolValue = *data->value.boolValue ? true : false;
			break;
		case CONTROL_VALUE_BYTE:
			*data->value.byteValue = (uint8_t)clamp(*data->value.byteValue, data->min, data->max);
			break;
		case CONTROL_VALUE_WORD:
			*data->value.wordValue = (uint16_t)clamp(*data->value.wordValue, data->min, data->max);
			break;
		case CONTROL_VALUE_DWORD:
			*data->value.dwordValue = (uint32_t)clamp(*data->value.dwordValue, data->min, data->max);
			break;
		case CONTROL_VALUE_FLOAT:
			*data->value.floatValue = clamp(*data->value.floatValue, data->min, data->max);
			break;
	}
}

static void SetSliderValue(SliderData *data, float newValue, const float roundTo)
{
	if (roundTo != 0)
	{
		newValue = roundf(newValue / roundTo) * roundTo;
	}

	newValue = clamp(newValue, data->min, data->max);

	switch (data->value.type)
	{
		case CONTROL_VALUE_BOOL:
			*data->value.boolValue = newValue > 0.0f;
			break;
		case CONTROL_VALUE_BYTE:
			*data->value.byteValue = (uint8_t)newValue;
			break;
		case CONTROL_VALUE_WORD:
			*data->value.wordValue = (uint16_t)newValue;
			break;
		case CONTROL_VALUE_DWORD:
			*data->value.dwordValue = (uint32_t)newValue;
			break;
		case CONTROL_VALUE_FLOAT:
			*data->value.floatValue = newValue;
			break;
	}

	if (data->callback != NULL)
	{
		data->callback(&data->value);
	}
}

static void ChangeSliderValue(SliderData *data, const float change, const float roundTo)
{
	float currentValue = GetSliderValueAsFloat(data);

	currentValue += change;
	if (currentValue < data->min)
	{
		currentValue = data->min;
	}
	if (currentValue > data->max)
	{
		currentValue = data->max;
	}

	SetSliderValue(data, currentValue, roundTo);
}

Control *CreateSliderControl(const Vector2 position,
							 const Vector2 size,
							 char *label,
							 const SliderCallback callback,
							 const ControlAnchor anchor,
							 const float min,
							 const float max,
							 const ControlValue value,
							 const float step,
							 const float altStep,
							 SliderLabelFunction getLabel,
							 char *tooltip)
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
	slider->tooltip = tooltip;

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

	ClampSliderValue(data);

	return slider;
}

void DestroySlider(const Control *c)
{
	SliderData *data = c->controlData;
	free(data);
}

void UpdateSlider(UiStack *stack, Control *c, Vector2 /*localMousePos*/, const uint32_t ctlIndex)
{
	SliderData *data = c->controlData;

	// handle l and r arrow keys
	if (stack->focusedControl == ctlIndex)
	{
		if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_LEFT) ||
			IsButtonJustPressed(mainThreadInput, SDL_GAMEPAD_BUTTON_DPAD_LEFT))
		{
			ConsumeKey(mainThreadInput, SDL_SCANCODE_LEFT);
			ConsumeButton(mainThreadInput, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
			ChangeSliderValue(data, -data->step, 0.0f);
		} else if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_RIGHT) ||
				   IsButtonJustPressed(mainThreadInput, SDL_GAMEPAD_BUTTON_DPAD_RIGHT))
		{
			ConsumeKey(mainThreadInput, SDL_SCANCODE_RIGHT);
			ConsumeButton(mainThreadInput, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
			ChangeSliderValue(data, data->step, 0.0f);
		}
	}

	if (stack->activeControl != ctlIndex)
	{
		return;
	}

	const bool pressed = IsMouseButtonPressed(mainThreadInput, SDL_BUTTON_LEFT);

	if (pressed)
	{
		const float newVal = remap(GetMousePos(mainThreadInput).x - c->anchoredPosition.x,
								   0.0,
									c->size.x,
									data->min,
									data->max);

		float step = data->step;
		if (IsKeyPressed(mainThreadInput, SDL_SCANCODE_LSHIFT) || IsKeyPressed(mainThreadInput, SDL_SCANCODE_RSHIFT))
		{
			step = data->altStep;
		}

		SetSliderValue(data, newVal, step);
	}

	ClampSliderValue(data);
}

void DrawSlider(const Control *c, const ControlState /*state*/, const Vector2 position)
{
	DrawNinePatchTexture(c->anchoredPosition, c->size, 8, 8, TEXTURE("interface/slider"));

	const SliderData *data = (SliderData *)c->controlData;
	const float handlePos = remap(GetSliderValueAsFloat(data), data->min, data->max, 0, c->size.x - 18);

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
					FONT("small_font"));
	DrawTextAligned(buf,
					16,
					COLOR_WHITE,
					position,
					c->size,
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					FONT("small_font"));
	free(buf);
}
