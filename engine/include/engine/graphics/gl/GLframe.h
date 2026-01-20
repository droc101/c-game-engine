//
// Created by droc101 on 11/20/25.
//

#ifndef GAME_GLFRAME_H
#define GAME_GLFRAME_H

#include <engine/graphics/RenderingHelpers.h> // NOLINT(*-include-cleaner) Required for the NDC macros
#include <stdbool.h>

/**
 * Create the OpenGL framebuffer and related data
 */
bool GL_InitFramebuffer();

/**
 * Destroy the OpenGL framebuffer and related data
 */
void GL_DestroyFramebuffer();

/**
 * Performs start of frame tasks
 */
bool GL_FrameStart();

/**
 * Clear the screen
 */
void GL_ClearScreen();

/**
 * Clear only the depth buffer
 */
void GL_ClearDepthOnly();

/**
 * Copy the internal framebuffer and swap buffers
 */
void GL_FrameEnd();

/**
 * Update the viewport and framebuffer sizes
 */
void GL_UpdateViewportSize();

/**
 * Convert screen X to NDC
 * @param x X position in pixels
 * @return The NDC position
 */
#define GL_X_TO_NDC(x) ((float)(x) / ScaledWindowWidthFloat() * 2.0f - 1.0f)

/**
 * Convert screen Y to NDC
 * @param y Y position in pixels
 * @return The NDC position
 */
#define GL_Y_TO_NDC(y) (1.0f - (float)(y) / ScaledWindowHeightFloat() * 2.0f)

#endif //GAME_GLFRAME_H
