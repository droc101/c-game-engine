//
// Created by droc101 on 7/20/26.
//

#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Color.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/VScrollBar.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


Control *CreateVScrollBarControl(Vector2 position, float height, ControlAnchor anchor, VScrollBarData *scrollBarData)
{
	Control *scrollbar = CreateEmptyControl();
	scrollbar->type = V_SCROLL_BAR;
	scrollbar->position = position;
	scrollbar->size = v2(16, height);
	scrollbar->anchor = anchor;
	CheckAlloc(scrollBarData);
	scrollbar->controlData = scrollBarData;

	return scrollbar;
}

void DestroyVScrollBar(const Control *) {}

void UpdateVScrollBar(UiStack *stack, Control *c, Vector2 /*localMousePos*/, uint32_t ctlIndex)
{
	VScrollBarData *data = (VScrollBarData *)c->controlData;
	if (stack->activeControl == ctlIndex && IsMouseButtonPressed(mainThreadInput, SDL_BUTTON_LEFT))
	{
		data->scrollPos -= (int)GetMouseRel(mainThreadInput).y;
	}
}

void DrawVScrollBar(const Control *c, const ControlState state, const Vector2 position)
{
	VScrollBarData *data = (VScrollBarData *)c->controlData;

	if (data->contentHeight > c->size.y)
	{
		data->scrollPos = clamp(data->scrollPos, -(data->contentHeight - c->size.y + 2), 0);
	} else
	{
		data->scrollPos = 0;
	}

	if ((float)data->contentHeight > c->size.y)
	{
		DrawNinePatchTexture(position, c->size, 2, 2, TEXTURE("interface/scrollbar"));

		const float percentVisible = (1.0f / (float)data->contentHeight) * c->size.y;
		const float percentScrolled = (1.0f / (float)data->contentHeight) * (float)abs(data->scrollPos);

		const Vector2 scrollbarPos = v2(position.x, position.y + c->size.y * percentScrolled);
		const Vector2 scrollbarSize = v2(c->size.x, c->size.y * percentVisible);

		const char *grabberTexture = TEXTURE("interface/scrollbar_grabber");
		if (state == HOVER)
		{
			grabberTexture = TEXTURE("interface/scrollbar_grabber_hover");
		} else if (state == ACTIVE)
		{
			grabberTexture = TEXTURE("interface/scrollbar_grabber_pressed");
		}
		DrawNinePatchTexture(scrollbarPos, scrollbarSize, 2, 2, grabberTexture);
	}
}
