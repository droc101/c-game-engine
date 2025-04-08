//
// Created by Noah on 11/9/2024.
//

// ReSharper disable CppUnusedIncludeDirective
#ifndef VULKANINTERNAL_H
#define VULKANINTERNAL_H

#include <cglm/cglm.h>

/**
 * This function will create the Vulkan instance, set up for SDL.
 * @see instance
 */
bool CreateInstance();

/**
 * Creates the Vulkan surface
 * @see surface
 */
bool CreateSurface();

bool CreateLogicalDevice();

bool CreateSwapChain();

bool CreateRenderPass();

bool CreateDescriptorSetLayouts();

bool CreateGraphicsPipelines();

bool InitTextures();

// bool CreateTexturesImageView();

bool CreateTextureSamplers();

bool CreateBuffers();

bool CreateDescriptorPool();

bool CreateDescriptorSets();

bool CreateCommandBuffers();

bool CreateSyncObjects();

#endif //VULKANINTERNAL_H
