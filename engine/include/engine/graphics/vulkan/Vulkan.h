//
// Created by Noah on 7/5/2024.
//

#ifndef GAME_VULKAN_H
#define GAME_VULKAN_H

#include <engine/graphics/Drawing.h>
#include <engine/graphics/RenderingHelpers.h> // NOLINT(*-include-cleaner)
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/Map.h>
#include <joltc/Math/Vector3.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef BUILDSTYLE_DEBUG
/**
 * Additional validation for Vulkan code
 * This will only work if the Vulkan SDK is installed on the device running the program.
 * @warning NOT FOR RELEASE BUILDS
 * @see https://docs.vulkan.org/guide/latest/validation_overview.html
 * @see https://vulkan.lunarg.com/doc/sdk/latest/windows/khronos_validation_layer.html
 */
#define VK_ENABLE_VALIDATION_LAYER
#endif

#define VK_X_TO_NDC(x) ((float)(x) / ScaledWindowWidthFloat() * 2.0f - 1.0f)
#define VK_Y_TO_NDC(y) ((float)(y) / ScaledWindowHeightFloat() * 2.0f - 1.0f)

bool VK_PreInit();

bool VK_Init(SDL_Window *window);

bool VK_FrameStart();

/**
 * Render a given map. This function will automatically load the map if it is not yet loaded, then it will update
 *  descriptor sets and push constants, before making the actual draw call for the level. This function does NOT submit
 *  any command buffers, present to the screen, or even begin the actual rendering process.
 * @param map The map to render
 * @param camera The camera from which the map should be rendered
 * @return @c VK_SUCCESS if the map was rendered successfully, or a meaningful result code otherwise
 */
bool VK_RenderMap(Map *map, const Camera *camera);

bool VK_FrameEnd();

/// A function used to destroy the Vulkan objects when they are no longer needed.
void VK_Cleanup();

bool VK_LoadMap(const Map *map);

bool VK_UpdateViewportSize();

void VK_Minimize();

void VK_Restore();

void VK_DrawColoredQuad(int x, int y, int w, int h, Color color);

void VK_DrawColoredQuadsBatched(const float *vertices, int quadCount, Color color);

void VK_DrawTexturedQuad(int x, int y, int w, int h, const char *texture);

void VK_DrawTexturedQuadMod(int x, int y, int w, int h, const char *texture, const Color *color);

void VK_DrawTexturedQuadRegion(int x,
							   int y,
							   int w,
							   int h,
							   int regionX,
							   int regionY,
							   int regionW,
							   int regionH,
							   const char *texture);

void VK_DrawTexturedQuadRegionMod(int x,
								  int y,
								  int w,
								  int h,
								  int regionX,
								  int regionY,
								  int regionW,
								  int regionH,
								  const char *texture,
								  Color color);

void VK_DrawTexturedQuadsBatched(const float *vertices, int quadCount, const char *texture, Color color);

void VK_DrawLine(int startX, int startY, int endX, int endY, int thickness, Color color);

void VK_DrawRectOutline(int x, int y, int w, int h, int thickness, Color color);

void VK_DrawUiTriangles(const UiTriangleArray *triangleArray, const char *texture, Color color);

void VK_DrawJoltDebugRendererLine(const Vector3 *from, const Vector3 *to, uint32_t color);

void VK_DrawJoltDebugRendererTriangle(const Vector3 *vertices, uint32_t color);

#endif //GAME_VULKAN_H
