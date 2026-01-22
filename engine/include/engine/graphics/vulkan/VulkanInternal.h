//
// Created by Noah on 11/9/2024.
//

// ReSharper disable CppUnusedIncludeDirective

#ifndef VULKANINTERNAL_H
#define VULKANINTERNAL_H

#include <SDL3/SDL_video.h>
#include <stdbool.h>

bool CreateInstance(SDL_Window *window);

bool CreateSurface();

bool CreateLogicalDevice();

bool CreateSwapchain();

bool CreateRenderPass();

bool CreateDescriptorSetLayouts();

bool CreateGraphicsPipelines();

bool CreateTextureSamplers();

bool CreateBuffers();

bool CreateDescriptorSets();

#endif //VULKANINTERNAL_H
