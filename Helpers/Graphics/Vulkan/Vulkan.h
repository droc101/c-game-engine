//
// Created by Noah on 7/5/2024.
//

#ifndef GAME_VULKAN_H
#define GAME_VULKAN_H

#include <vulkan/vulkan.h>
#include "../Drawing.h"
#include "../RenderingHelpers.h"

#define VK_X_TO_NDC(x) ((float)(x) / WindowWidth() * 2.0f - 1.0f)
#define VK_Y_TO_NDC(y) ((float)(y) / WindowHeight() * 2.0f - 1.0f)

/**
 * This function is used to create the Vulkan instance and surface, as well as configuring the environment properly.
 * This function (and the functions it calls) do NOT perform any drawing, though the framebuffers are initialized here.
 * @param window The window to initialize Vulkan for.
 * @see CreateInstance
 * @see PickPhysicalDevice
 * @see CreateLogicalDevice
 */
bool VK_Init(SDL_Window *window);

//TODO document me
VkResult VK_FrameStart();

//TODO document me
VkResult VK_FrameEnd();

//TODO document me
VkResult VK_RenderLevel(const Level *level, const Camera *camera);

/// A function used to destroy the Vulkan objects when they are no longer needed.
bool VK_Cleanup();

void VK_Minimize();

void VK_Restore();

bool VK_LoadLevelWalls(const Level *level);

void VK_DrawColoredQuad(int x, int y, int w, int h, Color color);

void VK_DrawColoredQuadsBatched(const float *vertices, int quadCount, Color color);

void VK_DrawTexturedQuad(int x, int y, int w, int h, const char *texture);

void VK_DrawTexturedQuadMod(int x, int y, int w, int h, const char *texture, Color color);

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

void VK_SetTexParams(const char *texture, bool linear, bool repeat);

#endif //GAME_VULKAN_H
