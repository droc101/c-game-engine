//
// Created by droc101 on 11/20/25.
//

#ifndef GAME_GLUI_H
#define GAME_GLUI_H

#include <cglm/types.h>
#include <engine/graphics/Drawing.h>
#include <engine/structs/Color.h>
#include <engine/structs/Vector2.h>
#include <GL/glew.h>
#include <stddef.h>
#include <stdint.h>

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
 * Draw a textured rectangle to the screen
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param texture The texture name
 * @param regionStart The start of the region in pixels
 * @param regionEnd The end of the region in pixels
 * @param color The modulate color
 */
void GL_DrawTexture_Internal(Vector2 pos,
							 Vector2 size,
							 const char *texture,
							 Color color,
							 Vector2 regionStart,
							 Vector2 regionEnd);

#endif //GAME_GLUI_H
