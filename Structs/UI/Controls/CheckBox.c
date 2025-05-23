//
// Created by droc101 on 10/27/2024.
//

#include "CheckBox.h"
#include "../../../Helpers/CommonAssets.h"
#include "../../../Helpers/Core/AssetReader.h"
#include "../../../Helpers/Core/Error.h"
#include "../../../Helpers/Core/Input.h"
#include "../../../Helpers/Graphics/Drawing.h"
#include "../../../Helpers/Graphics/Font.h"
#include "../../GlobalState.h"
#include "../../Vector2.h"

Control *CreateCheckboxControl(const Vector2 position,
							   const Vector2 size,
							   char *label,
							   const CheckboxCallback callback,
							   const ControlAnchor anchor,
							   const bool checked)
{
	Control *checkbox = CreateEmptyControl();
	checkbox->type = CHECKBOX;
	checkbox->position = position;
	checkbox->size = size;
	checkbox->anchor = anchor;

	checkbox->ControlData = malloc(sizeof(CheckBoxData));
	CheckAlloc(checkbox->ControlData);
	CheckBoxData *data = checkbox->ControlData;
	data->label = label;
	data->checked = checked;
	data->callback = callback;

	return checkbox;
}

void DestroyCheckbox(const Control *c)
{
	CheckBoxData *data = c->ControlData;
	free(data);
}

void UpdateCheckbox(UiStack *stack, Control *c, Vector2 /*localMousePos*/, uint /*ctlIndex*/)
{
	CheckBoxData *data = c->ControlData;

	if (HasActivation(stack, c))
	{
		PlaySoundEffect(SOUND("sfx_click"));
		data->checked = !data->checked;

		ConsumeMouseButton(SDL_BUTTON_LEFT);
		ConsumeKey(SDL_SCANCODE_SPACE);
		ConsumeButton(CONTROLLER_OK);

		if (data->callback != NULL)
		{
			data->callback(data->checked);
		}
	}
}

void DrawCheckbox(const Control *c, ControlState /*state*/, const Vector2 position)
{
	const CheckBoxData *data = (CheckBoxData *)c->ControlData;
	DrawTextAligned(data->label,
					16,
					COLOR_WHITE,
					v2(c->anchoredPosition.x + 40, c->anchoredPosition.y),
					v2(c->size.x - 40, c->size.y),
					FONT_HALIGN_LEFT,
					FONT_VALIGN_MIDDLE,
					smallFont);

	const Vector2 boxSize = v2s(32);
	const Vector2 boxPos = v2(position.x + 2, position.y + c->size.y / 2 - boxSize.y / 2);
	DrawTexture(boxPos,
				boxSize,
				data->checked ? TEXTURE("interface_checkbox_checked") : TEXTURE("interface_checkbox_unchecked"));
}
