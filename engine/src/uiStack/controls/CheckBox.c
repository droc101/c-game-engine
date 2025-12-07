//
// Created by droc101 on 10/27/2024.
//

#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/CheckBox.h>
#include <engine/uiStack/UiStack.h>
#include <SDL_mouse.h>
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

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

	checkbox->controlData = malloc(sizeof(CheckBoxData));
	CheckAlloc(checkbox->controlData);
	CheckBoxData *data = checkbox->controlData;
	data->label = label;
	data->checked = checked;
	data->callback = callback;

	return checkbox;
}

void DestroyCheckbox(const Control *c)
{
	CheckBoxData *data = c->controlData;
	free(data);
}

void UpdateCheckbox(UiStack *stack, Control *c, Vector2 /*localMousePos*/, uint32_t /*ctlIndex*/)
{
	CheckBoxData *data = c->controlData;

	if (HasActivation(stack, c))
	{
		(void)PlaySoundEffect(SOUND("sfx/click"), 0, 1, NULL, NULL);
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
	const CheckBoxData *data = (CheckBoxData *)c->controlData;
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
				data->checked ? TEXTURE("interface/checkbox_checked") : TEXTURE("interface/checkbox_unchecked"));
}
