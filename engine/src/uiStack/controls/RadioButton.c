//
// Created by droc101 on 10/27/2024.
//

#include <engine/uiStack/controls/RadioButton.h>
#include <SDL_mouse.h>
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <engine/assets/AssetReader.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/structs/List.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>

Control *CreateRadioButtonControl(const Vector2 position,
								  const Vector2 size,
								  char *label,
								  const RadioButtonCallback callback,
								  const ControlAnchor anchor,
								  const bool checked,
								  const uint8_t groupId,
								  const uint8_t id)
{
	Control *radio = CreateEmptyControl();
	radio->type = RADIO_BUTTON;
	radio->position = position;
	radio->size = size;
	radio->anchor = anchor;

	radio->controlData = malloc(sizeof(RadioButtonData));
	CheckAlloc(radio->controlData);
	RadioButtonData *data = radio->controlData;
	data->label = label;
	data->checked = checked;
	data->callback = callback;
	data->groupId = groupId;
	data->id = id;

	return radio;
}

void DestroyRadioButton(const Control *c)
{
	RadioButtonData *data = c->controlData;
	free(data);
}

void UpdateRadioButton(UiStack *stack, Control *c, Vector2 /*localMousePos*/, uint32_t /*ctlIndex*/)
{
	RadioButtonData *data = c->controlData;

	if (HasActivation(stack, c))
	{
		if (data->checked)
		{
			return; // do not allow re-checking
		}


		(void)PlaySoundEffect(SOUND("sfx/click"), 0, 1, NULL, NULL);
		data->checked = true;

		// Find all radio buttons with the same group id and uncheck them
		for (uint32_t i = 0; i < stack->controls.length; i++)
		{
			const Control *control = ListGetPointer(stack->controls, i);
			if (control->type == RADIO_BUTTON)
			{
				RadioButtonData *radioData = control->controlData;
				if (radioData->groupId == data->groupId && radioData->id != data->id)
				{
					radioData->checked = false;
				}
			}
		}

		ConsumeMouseButton(SDL_BUTTON_LEFT);
		ConsumeKey(SDL_SCANCODE_SPACE);
		ConsumeButton(CONTROLLER_OK);

		if (data->callback != NULL)
		{
			data->callback(data->checked, data->groupId, data->id);
		}
	}
}

void DrawRadioButton(const Control *c, ControlState /*state*/, const Vector2 position)
{
	const RadioButtonData *data = (RadioButtonData *)c->controlData;

	const Color textColor = data->checked ? COLOR_WHITE : COLOR(0xFFc0c0c0);

	DrawTextAligned(data->label,
					16,
					textColor,
					v2(c->anchoredPosition.x + 40, c->anchoredPosition.y),
					v2(c->size.x - 40, c->size.y),
					FONT_HALIGN_LEFT,
					FONT_VALIGN_MIDDLE,
					smallFont);

	const Vector2 boxSize = v2s(32);
	const Vector2 boxPos = v2(position.x + 2, position.y + c->size.y / 2 - boxSize.y / 2);
	DrawTexture(boxPos,
				boxSize,
				data->checked ? TEXTURE("interface/radio_checked") : TEXTURE("interface/radio_unchecked"));
}
