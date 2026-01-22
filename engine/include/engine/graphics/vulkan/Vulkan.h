//
// Created by Noah on 7/5/2024.
//

#ifndef GAME_VULKAN_H
#define GAME_VULKAN_H

#include <engine/graphics/Drawing.h>
#include <engine/graphics/RenderingHelpers.h> // NOLINT(*-include-cleaner)
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Viewmodel.h>
#include <joltc/Math/Vector3.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

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

/**
 * This function is used to create the Vulkan instance and surface, as well as configuring the environment properly.
 * This function (and the functions it calls) do NOT perform any drawing, though the framebuffers are initialized here.
 * @param window The window to initialize Vulkan for.
 * @see CreateInstance
 * @see PickPhysicalDevice
 * @see CreateLogicalDevice
 */
bool VK_Init(SDL_Window *window);

bool VK_LoadLevelWalls(const Map *level);

/// Update actors
bool VK_UpdateActors(const LockingList *actors, bool shouldReloadActors); // This is implemented in VulkanActors.c

//TODO document me
VkResult VK_FrameStart();

//TODO document me
VkResult VK_RenderLevel(const Map *level, const Camera *camera, const Viewmodel *viewmodel);

//TODO document me
VkResult VK_FrameEnd();

/// A function used to destroy the Vulkan objects when they are no longer needed.
bool VK_Cleanup();

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
