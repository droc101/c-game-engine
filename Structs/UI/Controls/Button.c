//
// Created by droc101 on 10/7/2024.
//

#include "Button.h"
#include "../../../Helpers/CommonAssets.h"
#include "../../../Helpers/Core/AssetReader.h"
#include "../../../Helpers/Core/Error.h"
#include "../../../Helpers/Core/Input.h"
#include "../../../Helpers/Graphics/Drawing.h"
#include "../../../Helpers/Graphics/Font.h"
#include "../../GlobalState.h"

Control *CreateButtonControl(const Vector2 position,
							 const Vector2 size,
							 char *text,
							 const ButtonCallback callback,
							 const ControlAnchor anchor)
{
	Control *btn = CreateEmptyControl();
	btn->type = BUTTON;
	btn->position = position;
	btn->size = size;
	btn->anchor = anchor;

	btn->ControlData = malloc(sizeof(ButtonData));
	CheckAlloc(btn->ControlData);
	ButtonData *data = btn->ControlData;
	data->text = text;
	data->callback = callback;
	data->enabled = true;

	return btn;
}

void DestroyButton(const Control *c)
{
	ButtonData *data = c->ControlData;
	free(data);
}

void UpdateButton(UiStack *stack, Control *c, Vector2 /*localMousePos*/, uint /*ctlIndex*/)
{
	const ButtonData *data = (ButtonData *)c->ControlData;
	if (data->enabled && HasActivation(stack, c))
	{
		PlaySoundEffect(SOUND("sfx/click"));
		ConsumeMouseButton(SDL_BUTTON_LEFT);
		ConsumeKey(SDL_SCANCODE_SPACE);
		ConsumeButton(CONTROLLER_OK);
		data->callback();
	}
}

void DrawButton(const Control *c, const ControlState state, const Vector2 position)
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

	const ButtonData *data = (ButtonData *)c->ControlData;

	DrawTextAligned(data->text, 16, COLOR_BLACK, position, c->size, FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, smallFont);
}
