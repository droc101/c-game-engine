//
// Created by droc101 on 7/31/26.
//

/**
 * @file IconButton.h
 * Button that shows an icon instead of text
 */

#ifndef GAME_ICONBUTTON_H
#define GAME_ICONBUTTON_H

#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct IconButtonData IconButtonData;

/**
 * IconButton clicked callback signature
 * @param iconButton The button that was clicked
 * @param extraData User data associated with the button
 */
typedef void (*IconButtonCallback)(Control *iconButton, void *extraData);

/// @private
struct IconButtonData
{
	char *icon;
	IconButtonCallback callback;
	bool enabled;
	void *extraData;
};

/**
 * Create an icon button control
 * @param position The position of the button
 * @param icon The icon of the button
 * @param callback The callback function to call when the button is clicked
 * @param anchor The anchor of the button
 * @param tooltip
 * @return The new IconButton Control
 */
Control *CreateIconButtonControl(Vector2 position,
								 char *icon,
								 IconButtonCallback callback,
								 ControlAnchor anchor,
								 char *tooltip);

/// @private
void DestroyIconButton(const Control *c);

/// @private
void UpdateIconButton(UiStack *stack, Control *c, Vector2 localMousePos, uint32_t ctlIndex);

/// @private
void DrawIconButton(const Control *c, ControlState state, Vector2 position);


#endif //GAME_ICONBUTTON_H
