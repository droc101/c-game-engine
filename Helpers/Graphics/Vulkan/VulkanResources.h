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

VkResult CreateWallActorBuffers();

VkResult CreateModelActorBuffers();

VkResult CreateRoofBuffers();

VkResult ResizeWallBuffers();

VkResult ResizeShadowBuffers();

VkResult ResizeWallActorBuffers();

VkResult ResizeModelActorBuffers();

bool LoadTexture(const Image *image);

#endif //VULKANRESOURCES_H
