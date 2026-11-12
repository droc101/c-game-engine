//
// Created by droc101 on 8/2/26.
//

/**
 * @file Image.h
 * Simple image control
 */

#ifndef GAME_IMAGE_H
#define GAME_IMAGE_H

#include <engine/structs/Vector2.h>
#include <engine/uiStack/UiStack.h>

typedef struct ImageData ImageData;

/// @private
struct ImageData
{
	const char *texture;
};


/**
 * Create a new Image control
 * @param position The position of the image
 * @param size The size of the image
 * @param texture The image to display
 * @param anchor The anchor of the image
 * @param tooltip The tooltip of the image
 * @return The new Image Control
 */
Control *CreateImageControl(Vector2 position, Vector2 size, const char *texture, ControlAnchor anchor, char *tooltip);

/// @private
void DestroyImage(const Control *c);

/// @private
void DrawImage(const Control *c, ControlState state, Vector2 position);

#endif //GAME_IMAGE_H
