//
// Created by droc101 on 7/31/26.
//

#include <engine/assets/FontLoader.h>
#include <engine/graphics/Font.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

Control *CreateLabelControl(char *str,
							const uint32_t size,
							const Color color,
							const Vector2 rectPos,
							const Vector2 rectSize,
							const ControlAnchor anchor,
							const FontHorizontalAlign hAlign,
							const FontVerticalAlign vAlign,
							const Font *font,
							const bool shadow)
{
	Control *label = CreateEmptyControl();
	label->type = LABEL;
	label->position = rectPos;
	label->size = rectSize;
	label->anchor = anchor;
	label->allowFocus = false;
	LabelControlData *data = malloc(sizeof(LabelControlData));
	CheckAlloc(data);
	data->size = size;
	data->color = color;
	data->font = font;
	data->hAlign = hAlign;
	data->vAlign = vAlign;
	data->str = str;
	data->shadow = shadow;
	label->controlData = data;

	return label;
}

void DestroyLabelControl(const Control *c)
{
	free(c->controlData);
}

void DrawLabelControl(const Control *c, ControlState /*state*/, Vector2 position)
{
	const LabelControlData *data = c->controlData;
	if (data->shadow)
	{
		DrawTextAligned(data->str,
						data->size,
						COLOR_BLACK,
						Vector2Add(v2s(2), position),
						c->size,
						data->hAlign,
						data->vAlign,
						data->font);
	}
	DrawTextAligned(data->str, data->size, data->color, position, c->size, data->hAlign, data->vAlign, data->font);
}
