//
// Created by droc101 on 10/7/2024.
//

#ifndef GAME_BUTTON_H
#define GAME_BUTTON_H

#include "../UiStack.h"

typedef struct ButtonData ButtonData;

typedef void (*ButtonCallback)();

struct ButtonData
{
	char *text;
	ButtonCallback callback;
	bool enabled;
};

/**
 * Create a new Button Control
 * @param position The position of the button
 * @param size The size of the button
 * @param text The text of the button
 * @param callback The callback function to call when the button is clicked
 * @param anchor The anchor of the button
 * @return The new Button Control
 */
Control *CreateButtonControl(Vector2 position, Vector2 size, char *text, ButtonCallback callback, ControlAnchor anchor);

void DestroyButton(const Control *c);

void UpdateButton(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

void DrawButton(const Control *c, ControlState state, Vector2 position);

#endif //GAME_BUTTON_H
