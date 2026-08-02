//
// Created by droc101 on 8/2/26.
//

#ifndef GAME_IMAGE_H
#define GAME_IMAGE_H

#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>

typedef struct ImageData ImageData;

struct ImageData
{
	const char *texture;
};


Control *CreateImageControl(Vector2 position, Vector2 size, const char *texture, ControlAnchor anchor, char *tooltip);

void DestroyImage(const Control *c);

void DrawImage(const Control *c, ControlState state, Vector2 position);

#endif //GAME_IMAGE_H
