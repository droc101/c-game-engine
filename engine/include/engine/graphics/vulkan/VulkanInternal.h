//
// Created by Noah on 11/9/2024.
//

// ReSharper disable CppUnusedIncludeDirective

#ifndef VULKANINTERNAL_H
#define VULKANINTERNAL_H

#include <SDL3/SDL_video.h>
#include <stdbool.h>

bool CreateInstance();

bool CreateSurface(SDL_Window *window);

bool CreateLogicalDevice();

bool CreateCommandBuffer();

bool CreateSwapchain();

bool CreateRenderPass();

bool CreateDescriptorSetLayouts();

bool CreateGraphicsPipelines();

bool CreateTextureSamplers();

bool CreateBuffers();

bool CreateDescriptorSet();

#endif //VULKANINTERNAL_H
