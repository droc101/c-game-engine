//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_DRAWING_H
#define GAME_DRAWING_H

#include "../../defines.h"
#include "SDL.h"

typedef struct BatchedQuadArray BatchedQuadArray;

struct BatchedQuadArray
{
	/// If used in a textured quad, @c verts takes the form of a @c float[quad_count * 16] holding values for X, Y, U, and V for each vertex.
	/// If used in a colored quad, @c verts takes the form of a @c float[quad_count * 8] holding values for X and Y for each vertex.
	float *verts;
	/// uint[quad_count * 6] with indices
	uint *indices;
	/// Number of quads in the array
	int quadCount;
};

/**
 * Draw a solid color rectangle
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param color The color to draw with
 * @note Call @c setColorUint before calling this function
 */
void DrawRect(int x, int y, int w, int h, Color color);

/**
 * Convert a texture asset to an SDL_Surface
 * @param texture The texture name
 * @param filterMode Texture filtering mode
 * @return The @c SDL_Surface
 */
SDL_Surface *ToSDLSurface(const char *texture, const char *filterMode);

/**
 * Draw a line from start to end
 * @param start The start of the line
 * @param end The end of the line
 * @param thickness The thickness of the line
 * @param color The color to draw with
 */
void DrawLine(const Vector2 start, const Vector2 end, const float thickness, const Color color);

/**
 * Draw a 1px outline of a rectangle
 * @param pos The position
 * @param size The size
 * @param thickness The thickness of the outline
 * @param color The color to draw with
 */
void DrawOutlineRect(const Vector2 pos, const Vector2 size, const float thickness, const Color color);

/**
 * Draw a texture on a rectangle
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @param texture The texture name
 */
void DrawTexture(Vector2 pos, Vector2 size, const char *texture);

/**
 * Draw a texture on a rectangle with a color
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @param texture The texture name
 * @param color The color to draw with
 */
void DrawTextureMod(Vector2 pos, Vector2 size, const char *texture, Color color);

/**
 * Draw a texture region on a rectangle
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @param texture The texture name
 * @param regionStart The start of the region (in pixels)
 * @param regionEnd The end of the region (in pixels)
 */
void DrawTextureRegion(Vector2 pos, Vector2 size, const char *texture, Vector2 regionStart, Vector2 regionEnd);

/**
 * Draw a texture region on a rectangle with a color
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @param texture The texture name
 * @param regionStart The start of the region (in pixels)
 * @param regionEnd The end of the region (in pixels)
 * @param color The color to draw with
 */
void DrawTextureRegionMod(Vector2 pos,
						  Vector2 size,
						  const char *texture,
						  Vector2 regionStart,
						  Vector2 regionEnd,
						  Color color);

/**
 * Clear the screen with the last used color
 */
void ClearScreen();

/**
 * Clear the depth buffer
 */
void ClearDepthOnly();

/**
 * Draw a nine patch image to the screen
 * @param pos The position to draw at
 * @param size The size of the output
 * @param outputMarginsPx The 9patch margins of the output
 * @param textureMarginsPx The 9patch margins of the texture
 * @param texture The texture name
 * @warning This is nine draw calls.
 */
void DrawNinePatchTexture(Vector2 pos,
						  Vector2 size,
						  float outputMarginsPx,
						  float textureMarginsPx,
						  const char *texture);

/**
 * Draw a `BatchedQuadArray` to the screen using the textured shader. This is faster than multiple draw calls, but harder to use.
 * @param batch The batch to draw
 * @param texture The texture name
 * @param color The color to use
 */
void DrawBatchedQuadsTextured(const BatchedQuadArray *batch, const char *texture, Color color);

/**
 * Draw a `BatchedQuadArray` to the screen using the solid color shader. This is faster than multiple draw calls, but harder to use.
 * @param batch The batch to draw
 * @param color The color to use
 */
void DrawBatchedQuadsColored(const BatchedQuadArray *batch, Color color);

/**
 * Draw a line. This function is intended to be called from Jolt's built-in debug renderer.
 * @param from The starting point of the line
 * @param to The ending point of the line
 * @param color The color of the line
 */
void DrawJoltDebugRendererDrawLine(void * /*userData*/, const JPH_RVec3 *from, const JPH_RVec3 *to, JPH_Color color);

/**
 * Draw a triangle. This function is intended to be called from Jolt's built-in debug renderer.
 * @param v1 The first vertex of the triangle
 * @param v2 The second vertex of the triangle
 * @param v3 The third vertex of the triangle
 * @param color The color of the triangle
 */
void DrawJoltDebugRendererDrawTriangle(void * /*userData*/,
									   const JPH_RVec3 *v1,
									   const JPH_RVec3 *v2,
									   const JPH_RVec3 *v3,
									   JPH_Color color,
									   JPH_DebugRenderer_CastShadow /*castShadow*/);

/**
 * Render the background of the menu screen (main menu, options, level select, etc.)
 */
void RenderMenuBackground();

/**
 * Render the background of the in-game menu (pause, in-game options, etc.)
 */
void RenderInGameMenuBackground();

/**
 * Render the 3D portion of a level
 * @param l The level to render
 * @param cam The camera to render with
 * @note - This does not render the sky
 * @note - This destroys the contents of the depth buffer
 */
void RenderLevel3D(const Level *l, const Camera *cam);

#endif //GAME_DRAWING_H
