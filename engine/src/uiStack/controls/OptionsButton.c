//
// Created by droc101 on 7/31/26.
//

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
#include <stdio.h>
#include <stdlib.h>

Control *CreateOptionsButtonControl(Vector2 position,
									Vector2 size,
									char *text,
									OptionsButtonCallback callback,
									ControlAnchor anchor,
									OptionsButtonValue *values,
									size_t numValues,
									void *extraData,
									size_t value)
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
	data->callback = callback;
	data->enabled = true;
	data->values = values;
	data->numValues = numValues;
	data->callbackExtraData = extraData;
	data->value = value;

	btn->tooltip = data->values[data->value].tooltip;

	return btn;
}

void DestroyOptionsButton(const Control *c)
{
	free(c->controlData);
}

void UpdateOptionsButton(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex)
{
	OptionsButtonData *data = (OptionsButtonData *)c->controlData;
	if (data->enabled && HasActivation(stack, c))
	{
		data->value++;
		if (data->value > data->numValues)
		{
			data->value = 0;
		}
		c->tooltip = data->values[data->value].tooltip;
		(void)PlaySound(SOUND("sfx/click"), SOUND_CATEGORY_UI);
		ConsumeMouseButton(mainThreadInput, SDL_BUTTON_LEFT);
		ConsumeKey(mainThreadInput, SDL_SCANCODE_SPACE);
		ConsumeButton(mainThreadInput, CONTROLLER_OK);
		data->callback(data->value, data->callbackExtraData);
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
	snprintf(label, 256, data->text, data->values[data->value].text);

	DrawTextAligned(label, 16, COLOR_BLACK, position, c->size, FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, smallFont);
}
