//
// Created by droc101 on 10/27/2024.
//

#include "RadioButton.h"
#include "../../../Helpers/CommonAssets.h"
#include "../../../Helpers/Core/AssetReader.h"
#include "../../../Helpers/Core/Error.h"
#include "../../../Helpers/Core/Input.h"
#include "../../../Helpers/Graphics/Drawing.h"
#include "../../../Helpers/Graphics/Font.h"
#include "../../GlobalState.h"
#include "../../Vector2.h"

Control *CreateRadioButtonControl(const Vector2 position,
								  const Vector2 size,
								  char *label,
								  const RadioButtonCallback callback,
								  const ControlAnchor anchor,
								  const bool checked,
								  const byte groupId,
								  const byte id)
{
	Control *radio = CreateEmptyControl();
	radio->type = RADIO_BUTTON;
	radio->position = position;
	radio->size = size;
	radio->anchor = anchor;

	radio->ControlData = malloc(sizeof(RadioButtonData));
	CheckAlloc(radio->ControlData);
	RadioButtonData *data = radio->ControlData;
	data->label = label;
	data->checked = checked;
	data->callback = callback;
	data->groupId = groupId;
	data->id = id;

	return radio;
}

void DestroyRadioButton(const Control *c)
{
	RadioButtonData *data = c->ControlData;
	free(data);
}

void UpdateRadioButton(UiStack *stack, Control *c, Vector2 /*localMousePos*/, uint /*ctlIndex*/)
{
	RadioButtonData *data = c->ControlData;

	if (HasActivation(stack, c))
	{
		if (data->checked)
		{
			return; // do not allow re-checking
		}


		PlaySoundEffect(SOUND("sfx_click"));
		data->checked = true;

		// Find all radio buttons with the same group id and uncheck them
		for (uint i = 0; i < stack->Controls.length; i++)
		{
			const Control *control = ListGetPointer(stack->Controls, i);
			if (control->type == RADIO_BUTTON)
			{
				RadioButtonData *radioData = control->ControlData;
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
	const RadioButtonData *data = (RadioButtonData *)c->ControlData;

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
				data->checked ? TEXTURE("interface_radio_checked") : TEXTURE("interface_radio_unchecked"));
}
