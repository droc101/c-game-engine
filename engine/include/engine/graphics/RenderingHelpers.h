//
// Created by droc101 on 10/2/24.
//

#ifndef GAME_RENDERINGHELPERS_H
#define GAME_RENDERINGHELPERS_H

#include <cglm/types.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Color.h>
#include <engine/structs/Map.h>
#include <engine/structs/Vector2.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stdint.h>

/// Field of view used for viewmodels
#define VIEWMODEL_FOV 70
/// Near clipping plane
#define NEAR_Z 0.01
/// Far clipping plane
#define FAR_Z 1000

// Default window size
#define DEF_WIDTH 1280
#define DEF_HEIGHT 720

// Minimum and maximum window size
#define MIN_WIDTH 960
#define MIN_HEIGHT 720

// Maximum window size
#define MAX_WIDTH 9999
#define MAX_HEIGHT 9999

typedef enum Renderer Renderer;

/**
 * Used to check which renderer the game is using
 */
enum Renderer
{
	RENDERER_OPENGL,
	RENDERER_VULKAN,
	RENDERER_MAX
};

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
 * Get the width of the window divided by the ui scale
 * @return width of the window divided by the ui scale
 */
int ScaledWindowWidth();

/**
 * Get the height of the window divided by the ui scale
 * @return height of the window divided by the ui scale
 */
int ScaledWindowHeight();

/**
 * Get the width of the window divided by the ui scale
 * @return width of the window divided by the ui scale
 */
float ScaledWindowWidthFloat();

/**
 * Get the height of the window divided by the ui scale
 * @return height of the window divided by the ui scale
 */
float ScaledWindowHeightFloat();

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
 * Convert a color uint32_t (0xAARRGGBB) to a Color vec4 (RGBA 0-1)
 * @param argb The color uint32_t
 * @param color The output color
 */
void GetColor(uint32_t argb, Color *color);

#endif //GAME_RENDERINGHELPERS_H
