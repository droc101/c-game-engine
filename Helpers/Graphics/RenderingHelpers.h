//
// Created by droc101 on 10/2/24.
//

#ifndef GAME_RENDERINGHELPERS_H
#define GAME_RENDERINGHELPERS_H

#include "../../defines.h"

extern Renderer currentRenderer;


/**
 * Set the main window
 * @param w The window to use
 */
void SetGameWindow(SDL_Window *w);

/**
 * Get the main window
 * @return the window
 */
SDL_Window *GetGameWindow();

/**
 * Get the width of the window
 * @return width of the window
 */
int WindowWidth();

/**
 * Get the height of the window
 * @return height of the window
 */
int WindowHeight();

/**
 * Get the width of the window
 * @return width of the window
 */
float WindowWidthFloat();

/**
 * Get the height of the window
 * @return height of the window
 */
float WindowHeightFloat();

/**
 * Updates the variables returned by @c WindowWidth() and @c WindowHeight()
 */
void UpdateWindowSize();

/**
 * Get the actual size of the window, ignoring UI scale
 * @return The actual size of the window
 */
Vector2 ActualWindowSize();

/**
 * Get the size of a texture
 * @param texture The texture name
 * @return The size of the texture
 */
Vector2 GetTextureSize(const char *texture);

/**
 * Get the transformation matrix for an actor
 * @param actor The actor
 * @param transformMatrix A mat4 MODEL matrix of the actor (Model space to world space)
 */
void ActorTransformMatrix(const Actor *actor, mat4 *transformMatrix);

/**
 * Perform any pre-initialization for the rendering system
 * This is called before the window is created.
 */
bool RenderPreInit();

/**
 * Initialize the rendering system
 */
bool RenderInit();

/**
 * Destroy the rendering system
 */
void RenderDestroy();

/**
 * Run tasks that need to be run before any drawing can be done
 */
bool FrameStart();

/**
 * Run tasks needed to present the frame to the screen, as well as swapping the framebuffers
 */
void FrameEnd();

void LoadLevelWalls(const Level *l);

/**
 * Update the viewport size
 */
void UpdateViewportSize();

/**
 * Handle minimizing the window
 */
void WindowObscured();

/**
 * Handle restoring the window from minimized state
 */
void WindowRestored();

/**
 * Enable or disable low FPS mode
 * @param val A boolean representing if low FPS mode should be enabled
 */
void SetLowFPS(bool val);

/**
 * Check if low FPS mode is enabled
 * @return A boolean representing if low FPS is enabled
 */
bool IsLowFPSModeEnabled();

/**
 * Convert screen X to NDC
 * @param x X position in pixels
 * @return The NDC position
 */
float X_TO_NDC(float x);

/**
 * Convert screen Y to NDC
 * @param y Y position in pixels
 * @return The NDC position
 */
float Y_TO_NDC(float y);

/**
 * Convert a color uint (0xAARRGGBB) to a Color vec4 (RGBA 0-1)
 * @param argb The color uint
 * @param color The output color
 */
void GetColor(const uint argb, Color *color);

#endif //GAME_RENDERINGHELPERS_H
