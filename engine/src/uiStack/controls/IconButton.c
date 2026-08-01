//
// Created by droc101 on 7/31/26.
//

#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/IconButton.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

Control *CreateIconButtonControl(Vector2 position,
								 char *icon,
								 IconButtonCallback callback,
								 ControlAnchor anchor,
								 char *tooltip)
{
	Control *btn = CreateEmptyControl();
	btn->type = ICON_BUTTON;
	btn->position = position;
	btn->size = v2s(40);
	btn->anchor = anchor;
	btn->tooltip = tooltip;

	btn->controlData = malloc(sizeof(IconButtonData));
	CheckAlloc(btn->controlData);
	IconButtonData *data = btn->controlData;
	data->icon = icon;
	data->callback = callback;
	data->enabled = true;
	data->extraData = NULL;

	return btn;
}

void DestroyIconButton(const Control *c)
{
	free(c->controlData);
}

void UpdateIconButton(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex)
{
	const IconButtonData *data = (IconButtonData *)c->controlData;
	if (data->enabled && HasActivation(stack, c))
	{
		(void)PlaySound(SOUND("sfx/click"), SOUND_CATEGORY_UI);
		ConsumeMouseButton(mainThreadInput, SDL_BUTTON_LEFT);
		ConsumeKey(mainThreadInput, SDL_SCANCODE_SPACE);
		ConsumeButton(mainThreadInput, CONTROLLER_OK);
		data->callback(c, data->extraData);
	}
}

void DrawIconButton(const Control *c, ControlState state, Vector2 position)
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

	const IconButtonData *data = (IconButtonData *)c->controlData;
	DrawTexture(position, c->size, data->icon);
}
