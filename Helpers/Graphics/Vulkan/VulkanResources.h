//
// Created by Noah on 12/18/2024.
//

#ifndef VULKANRESOURCES_H
#define VULKANRESOURCES_H

#include <vulkan/vulkan.h>
#include "../../../defines.h"

VkResult CreateUiBuffers();

VkResult CreateViewModelBuffers();

VkResult CreateWallBuffers();

VkResult CreateActorWallBuffers();

VkResult CreateActorModelBuffers();

VkResult ResizeWallBuffers();

VkResult ResizeActorWallBuffers();

VkResult ResizeActorModelBuffers();

bool LoadTexture(const Image *image);

#endif //VULKANRESOURCES_H
