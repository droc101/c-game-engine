//
// Created by droc101 on 7/31/26.
//

#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/uiStack/controls/HeaderFooterControl.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Control *CreateHeaderFooterControl(const float height, const bool isHeader, char *label)
{
	Control *header = CreateEmptyControl();
	header->type = HEADER_FOOTER;
	header->position = v2s(0);
	header->size = v2(ScaledWindowWidthFloat(), height);
	header->anchor = isHeader ? TOP_LEFT : BOTTOM_LEFT;
	header->allowFocus = false;
	HeaderFooterControlData *data = malloc(sizeof(HeaderFooterControlData));
	CheckAlloc(data);
	data->isHeader = isHeader;
	if (label)
	{
		data->label = strdup(label);
		CheckAlloc(label);
	} else
	{
		data->label = NULL;
	}
	header->controlData = data;

	return header;
}

void DestroyHeaderFooterControl(const Control *c)
{
	const HeaderFooterControlData *data = c->controlData;
	free(data->label);
	free(data);
}

void AlwaysUpdateHeaderFooterControl(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex)
{
	c->size.x = ScaledWindowWidthFloat();
}

void DrawHeaderFooterControl(const Control *c, ControlState state, Vector2 position)
{
	HeaderFooterControlData *data = c->controlData;

	DrawNinePatchTexture(position,
						 c->size,
						 4,
						 4,
						 data->isHeader ? TEXTURE("interface/header") : TEXTURE("interface/footer"));

	if (data->label)
	{
		DrawTextAligned(data->label,
						32,
						COLOR_WHITE,
						position,
						c->size,
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						FONT("large_font"));
	}
}
