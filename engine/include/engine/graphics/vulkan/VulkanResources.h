//
// Created by Noah on 12/18/2024.
//

#ifndef VULKANRESOURCES_H
#define VULKANRESOURCES_H

#include <stdbool.h>
#include <vulkan/vulkan_core.h>
#include <engine/assets/TextureLoader.h>

VkResult CreateUiBuffers();

VkResult CreateViewModelBuffers();

VkResult CreateWallBuffers();

VkResult CreateActorWallBuffers();

VkResult CreateActorModelBuffers();

VkResult CreateDebugDrawBuffers();

VkResult ResizeWallBuffers();

VkResult ResizeActorWallBuffers();

VkResult ResizeActorModelInstanceDataBuffer();

VkResult ResizeActorModelBuffers();

VkResult ResizeDebugDrawBuffers();

bool LoadTexture(const Image *image);

#endif //VULKANRESOURCES_H
