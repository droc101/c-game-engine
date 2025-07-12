//
// Created by Noah on 12/18/2024.
//

#ifndef VULKANRESOURCES_H
#define VULKANRESOURCES_H

#include <vulkan/vulkan.h>
#include "../../../defines.h"

VkResult CreateUiBuffers();

VkResult CreateWallBuffers();

VkResult CreateShadowBuffers();

VkResult CreateActorWallBuffers();

VkResult CreateActorModelBuffers();

VkResult CreateRoofBuffers();

VkResult ResizeWallBuffers();

VkResult ResizeShadowBuffers();

VkResult ResizeActorWallBuffers();

VkResult ResizeActorModelBuffers();

bool LoadTexture(const Image *image);

#endif //VULKANRESOURCES_H
