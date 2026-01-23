//
// Created by droc101 on 10/7/2024.
//

#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

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
		(void)PlaySound(SOUND("sfx/click"), SOUND_CATEGORY_UI);
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
