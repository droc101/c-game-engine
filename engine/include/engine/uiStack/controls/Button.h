//
// Created by droc101 on 10/7/2024.
//

/**
 * @file Button.h
 * Simple button control
 */

#ifndef GAME_BUTTON_H
#define GAME_BUTTON_H

#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct ButtonData ButtonData;

/**
 * Button click callback signature
 * @param button The button that was clicked
 * @param extraData User data associated with the button
 */
typedef void (*ButtonCallback)(Control *button, void *extraData);

/// @private
struct ButtonData
{
	/// The text of the button
	const char *text;
	/// The function to call when the button is clicked
	ButtonCallback callback;
	/// Whether the button is enabled
	bool enabled;
	/// Extra data to send with the click callback
	void *extraData;
};

/**
 * Create a new Button Control
 * @param position The position of the button
 * @param size The size of the button
 * @param text The text of the button
 * @param callback The callback function to call when the button is clicked
 * @param anchor The anchor of the button
 * @param tooltip
 * @return The new Button Control
 */
Control *CreateButtonControl(Vector2 position,
							 Vector2 size,
							 const char *text,
							 ButtonCallback callback,
							 ControlAnchor anchor,
							 char *tooltip);

/// @private
void DestroyButton(const Control *c);

/// @private
void UpdateButton(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

/// @private
void DrawButton(const Control *c, ControlState state, Vector2 position);

#endif //GAME_BUTTON_H
