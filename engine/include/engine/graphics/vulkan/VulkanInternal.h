//
// Created by Noah on 11/9/2024.
//

// ReSharper disable CppUnusedIncludeDirective

#ifndef VULKANINTERNAL_H
#define VULKANINTERNAL_H

#include <SDL3/SDL_video.h>
#include <stdbool.h>

void CreateInstance();

void CreateSurface(SDL_Window *window);

bool CreateLogicalDevice();

void CreateCommandBuffers();

void CreateSwapchain();

void CreateRenderPass();

void CreateDescriptorSetLayouts();

void CreateComputePipelines();

void CreateGraphicsPipelines();

void UpdateLightCount();

void CreateTextureSamplers();

void CreateBuffers();

void CreateDescriptorSet();

void WriteDescriptorSet();

#endif //VULKANINTERNAL_H
