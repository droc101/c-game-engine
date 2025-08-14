//
// Created by droc101 on 10/7/2024.
//

#include "Button.h"
#include <SDL_mouse.h>
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../../Helpers/CommonAssets.h"
#include "../../../Helpers/Core/AssetReader.h"
#include "../../../Helpers/Core/Error.h"
#include "../../../Helpers/Core/Input.h"
#include "../../../Helpers/Core/SoundSystem.h"
#include "../../../Helpers/Graphics/Drawing.h"
#include "../../../Helpers/Graphics/Font.h"
#include "../../Color.h"
#include "../../Vector2.h"
#include "../UiStack.h"

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

	btn->controlData = malloc(sizeof(ButtonData));
	CheckAlloc(btn->controlData);
	ButtonData *data = btn->controlData;
	data->text = text;
	data->callback = callback;
	data->enabled = true;

	return btn;
}

void DestroyButton(const Control *c)
{
	ButtonData *data = c->controlData;
	free(data);
}

void UpdateButton(UiStack *stack, Control *c, Vector2 /*localMousePos*/, uint32_t /*ctlIndex*/)
{
	const ButtonData *data = (ButtonData *)c->controlData;
	if (data->enabled && HasActivation(stack, c))
	{
		(void)PlaySoundEffect(SOUND("sfx/click"), 0, 1, NULL, NULL);
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

	const ButtonData *data = (ButtonData *)c->controlData;

	DrawTextAligned(data->text, 16, COLOR_BLACK, position, c->size, FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, smallFont);
}
