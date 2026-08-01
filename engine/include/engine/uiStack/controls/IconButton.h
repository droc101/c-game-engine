//
// Created by droc101 on 7/31/26.
//

#ifndef GAME_ICONBUTTON_H
#define GAME_ICONBUTTON_H

#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct IconButtonData IconButtonData;

typedef void (*IconButtonCallback)(Control *iconButton, void *extraData);

struct IconButtonData
{
	char *icon;
	IconButtonCallback callback;
	bool enabled;
	void *extraData;
};

Control *CreateIconButtonControl(Vector2 position,
								 char *icon,
								 IconButtonCallback callback,
								 ControlAnchor anchor,
								 char *tooltip);

void DestroyIconButton(const Control *c);

void UpdateIconButton(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

void DrawIconButton(const Control *c, ControlState state, Vector2 position);


#endif //GAME_ICONBUTTON_H
