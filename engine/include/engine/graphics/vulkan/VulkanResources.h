//
// Created by Noah on 12/18/2024.
//

#ifndef VULKANRESOURCES_H
#define VULKANRESOURCES_H

#include <engine/assets/TextureLoader.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

VkResult CreateUiBuffers();

VkResult CreateUniformBuffers();

VkResult CreateMapBuffers();

VkResult CreateSkyBuffers();

VkResult CreateViewmodelBuffers();

VkResult CreateActorModelBuffers();

VkResult CreateDebugDrawBuffers();

VkResult ResizeDebugDrawBuffers();

bool LoadTexture(const Image *image);

#endif //VULKANRESOURCES_H
