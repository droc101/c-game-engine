//
// Created by droc101 on 9/30/2024.
//

#ifndef GAME_GLHELPER_H
#define GAME_GLHELPER_H

#include <cglm/types.h>
#include <engine/assets/ModelLoader.h>
#include <engine/graphics/Drawing.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/Map.h>
#include <engine/structs/Vector2.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stdint.h>

#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 3
#define GL_PROFILE SDL_GL_CONTEXT_PROFILE_CORE
#define GL_VERSION_CHECK GLEW_VERSION_3_3
#define GL_VERSION_STRING "OpenGL 3.3 Core Profile"
#define GL_INIT_FAIL_MSG "Failed to start OpenGL. Your GPU or drivers may not support the " GL_VERSION_STRING "."

/**
 * Set SDL_GL flags (this must be done before the SDL window is created)
 */
bool GL_PreInit();

/**
 * Initialize OpenGL
 */
bool GL_Init(SDL_Window *wnd);

/**
 * Clear the screen
 */
void GL_ClearScreen();

/**
 * Clear only the depth buffer
 */
void GL_ClearDepthOnly();

/**
 * Swap the buffers
 */
void GL_Swap();

/**
 * Destroy the GL state
 */
void GL_DestroyGL();

/**
 * Draw a rectangle
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param color The color to draw
 */
void GL_DrawRect(Vector2 pos, Vector2 size, Color color);

/**
 * Draw a rectangle outline
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param color The color to draw
 * @param thickness The thickness of the outline
 */
void GL_DrawRectOutline(Vector2 pos, Vector2 size, Color color, float thickness);

/**
 * Draw a line
 * @param start The start position in pixels
 * @param end The end position in pixels
 * @param color The color to draw
 * @param thickness The thickness of the line
 */
void GL_DrawLine(Vector2 start, Vector2 end, Color color, float thickness);

/**
 * Draw a texture in 2D
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param texture The texture name
 */
void GL_DrawTexture(Vector2 pos, Vector2 size, const char *texture);

/**
 * Draw a texture in 2D with a color mod
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param texture The texture name
 * @param color The modulate color
 */
void GL_DrawTextureMod(Vector2 pos, Vector2 size, const char *texture, Color color);

/**
 * Draw a texture region in 2D
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param texture The texture name
 * @param regionStart The start of the region in pixels
 * @param regionEnd The end of the region in pixels
 */
void GL_DrawTextureRegion(Vector2 pos, Vector2 size, const char *texture, Vector2 regionStart, Vector2 regionEnd);

/**
 * Draw a texture region in 2D with a color mod
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param texture The texture name
 * @param regionStart The start of the region in pixels
 * @param regionEnd The end of the region in pixels
 * @param color The modulate color
 */
void GL_DrawTextureRegionMod(Vector2 pos,
							 Vector2 size,
							 const char *texture,
							 Vector2 regionStart,
							 Vector2 regionEnd,
							 Color color);

/**
 * Update the viewport size
 */
void GL_UpdateViewportSize();

/**
 * Draw arrays using the ui_textured shader
 * @param vertices Vertex data [x, y, u, v] with UVs in NDC
 * @param indices Index data
 * @param quadCount The number of quads to draw
 * @param texture The texture name
 * @param color The modulate color
 */
void GL_DrawTexturedArrays(const float *vertices,
						   const uint32_t *indices,
						   int quadCount,
						   const char *texture,
						   Color color);

/**
 * Draw a `UITriangleArray` using the textured shader.
 * @param tris The triangles to draw
 * @param texture The texture name
 * @param col The color to use
 */
void GL_DrawUITriangles(const UiTriangleArray *tris, const char *texture, Color col);

/**
 * Draw arrays using the ui_colored shader
 * @param vertices Vertex data [x, y] with positions in NDC
 * @param indices Index data
 * @param quadCount The number of quads to draw
 * @param color The color to draw
 */
void GL_DrawColoredArrays(const float *vertices, const uint32_t *indices, uint32_t quadCount, Color color);

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

/**
 * Get the transformation matrix for a camera
 * @param camera The camera
 * @param modelViewProjectionMatrix
 * @return A mat4 MODEL_VIEW_PROJECTION matrix of the camera (World space to screen space)
 */
void GL_GetMatrix(const Camera *camera, mat4 *modelViewProjectionMatrix);

/**
 * Get the transform matrix for the viewmodel/held item
 * @param out The destination matrix
 */
void GL_GetViewmodelMatrix(mat4 *out);

/**
 * OpenGL code to render the 3D portion of a level
 * @param level The level to render
 * @param camera The camera to render with
 * @note - This does not render the sky
 * @note - This destroys the contents of the depth buffer
 */
void GL_RenderLevel(const Map *level, const Camera *camera);

/**
 * Render a 3D model
 * @param model The model to render
 * @param modelWorldMatrix The model -> world matrix
 * @param skin The skin to use
 * @param lod The lod to use
 * @param modColor
 */
void GL_RenderModel(const ModelDefinition *model,
					const mat4 modelWorldMatrix,
					uint32_t skin,
					uint32_t lod,
					Color modColor);

#endif //GAME_GLHELPER_H
