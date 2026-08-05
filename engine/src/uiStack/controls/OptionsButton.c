//
// Created by droc101 on 7/31/26.
//

#include <assert.h>
#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/OptionsButton.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static size_t FindCurrentValueIndex(const OptionsButtonData *data)
{
	for (size_t i = 0; i < data->numValues; i++)
	{
		const LiteralControlValue *current = &data->values[i].value;
		bool same = false;
		switch (data->value.type)
		{
			case CONTROL_VALUE_BOOL:
				same = *data->value.boolValue == current->boolValue;
				break;
			case CONTROL_VALUE_BYTE:
				same = *data->value.byteValue == current->byteValue;
				break;
			case CONTROL_VALUE_WORD:
				same = *data->value.wordValue == current->wordValue;
				break;
			case CONTROL_VALUE_DWORD:
				same = *data->value.dwordValue == current->dwordValue;
				break;
			case CONTROL_VALUE_FLOAT:
				same = *data->value.floatValue == current->floatValue;
				break;
		}
		if (same)
		{
			return i;
		}
	}
	return 0;
}

static const OptionsButtonValue *FindCurrentValue(const OptionsButtonData *data)
{
	return &data->values[FindCurrentValueIndex(data)];
}

Control *CreateOptionsButtonControl(const Vector2 position,
									const Vector2 size,
									char *text,
									const OptionsButtonCallback callback,
									const ControlAnchor anchor,
									OptionsButtonValue *values,
									const size_t numValues,
									void *extraData,
									const ControlValue value,
									char *alwaysTooltip)
{
	Control *btn = CreateEmptyControl();
	btn->type = OPTIONS_BUTTON;
	btn->position = position;
	btn->size = size;
	btn->anchor = anchor;

	btn->controlData = malloc(sizeof(OptionsButtonData));
	CheckAlloc(btn->controlData);
	OptionsButtonData *data = btn->controlData;
	data->text = text;
	data->alwaysTooltip = alwaysTooltip;
	data->callback = callback;
	data->enabled = true;
	data->values = values;
	data->numValues = numValues;
	data->callbackExtraData = extraData;
	data->value = value;

	for (size_t i = 0; i < numValues; i++)
	{
		assert(values[i].value.type == value.type);
	}

	if (data->alwaysTooltip)
	{
		btn->tooltip = data->alwaysTooltip;
	} else
	{
		btn->tooltip = FindCurrentValue(data)->tooltip;
	}

	return btn;
}

void DestroyOptionsButton(const Control *c)
{
	free(c->controlData);
}

void UpdateOptionsButton(UiStack *stack, Control *c, Vector2 /*localMousePos*/, uint32_t /*ctlIndex*/)
{
	OptionsButtonData *data = (OptionsButtonData *)c->controlData;
	if (data->enabled && HasActivation(stack, c))
	{
		size_t index = FindCurrentValueIndex(data);
		index++;
		if (index >= data->numValues)
		{
			index = 0;
		}
		if (!data->alwaysTooltip)
		{
			c->tooltip = data->values[index].tooltip;
		}
		const LiteralControlValue *value = &data->values[index].value;
		switch (data->value.type)
		{
			case CONTROL_VALUE_BOOL:
				*data->value.boolValue = value->boolValue;
				break;
			case CONTROL_VALUE_BYTE:
				*data->value.byteValue = value->byteValue;
				break;
			case CONTROL_VALUE_WORD:
				*data->value.wordValue = value->wordValue;
				break;
			case CONTROL_VALUE_DWORD:
				*data->value.dwordValue = value->dwordValue;
				break;
			case CONTROL_VALUE_FLOAT:
				*data->value.floatValue = value->floatValue;
				break;
		}
		data->callback(&data->values[index], data->callbackExtraData);
		(void)PlaySound(SOUND("sfx/click"), SOUND_CATEGORY_UI);
		ConsumeMouseButton(mainThreadInput, SDL_BUTTON_LEFT);
		ConsumeKey(mainThreadInput, SDL_SCANCODE_SPACE);
		ConsumeButton(mainThreadInput, CONTROLLER_OK);
	}
}

void DrawOptionsButton(const Control *c, ControlState state, Vector2 position)
{
	switch (state)
	{
		case NORMAL:
			DrawNinePatchTexture(c->anchoredPosition, c->size, 8, 8, TEXTURE("interface/button"));
			break;
		case HOVER:
			DrawNinePatchTexture(c->anchoredPosition, c->size, 8, 8, TEXTURE("interface/button_hover"));
			break;
		case ACTIVE:
			DrawNinePatchTexture(c->anchoredPosition, c->size, 8, 8, TEXTURE("interface/button_pressed"));
			break;
	}

	const OptionsButtonData *data = (OptionsButtonData *)c->controlData;

	char label[256] = {0};
	snprintf(label, 256, data->text, FindCurrentValue(data)->text);

	DrawTextAligned(label,
					16,
					COLOR_BLACK,
					position,
					c->size,
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					FONT("small_font"));
}

OptionsButtonValue onOffButtonValues[2] = {
	{
		.text = "Off",
		.tooltip = NULL,
		.value =
				{
					.type = CONTROL_VALUE_BOOL,
					.boolValue = false,
				},
	},
	{
		.text = "On",
		.tooltip = NULL,
		.value =
				{
					.type = CONTROL_VALUE_BOOL,
					.boolValue = true,
				},
	},
};

OptionsButtonValue yesNoButtonValues[2] = {
	{
		.text = "No",
		.tooltip = NULL,
		.value =
				{
					.type = CONTROL_VALUE_BOOL,
					.boolValue = false,
				},
	},
	{
		.text = "Yes",
		.tooltip = NULL,
		.value =
				{
					.type = CONTROL_VALUE_BOOL,
					.boolValue = true,
				},
	},
};
