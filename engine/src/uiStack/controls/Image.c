//
// Created by droc101 on 8/2/26.
//

#include <engine/graphics/Drawing.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/uiStack/controls/Image.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

Control *CreateImageControl(const Vector2 position,
							const Vector2 size,
							const char *texture,
							const ControlAnchor anchor,
							char *tooltip)
{
	Control *img = CreateEmptyControl();
	img->type = IMAGE;
	img->position = position;
	img->size = size;
	img->anchor = anchor;
	img->tooltip = tooltip;
	img->allowFocus = false;

	img->controlData = malloc(sizeof(ImageData));
	CheckAlloc(img->controlData);
	ImageData *data = img->controlData;
	data->texture = texture;

	return img;
}

void DestroyImage(const Control *c)
{
	free(c->controlData);
}

void DrawImage(const Control *c, ControlState state, Vector2 position)
{
	ImageData *data = c->controlData;
	DrawTexture(position, c->size, data->texture);
}