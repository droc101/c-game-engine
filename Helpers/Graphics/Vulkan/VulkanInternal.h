//
// Created by Noah on 11/9/2024.
//

// ReSharper disable CppUnusedIncludeDirective
#ifndef VULKANINTERNAL_H
#define VULKANINTERNAL_H

#include <cglm/cglm.h>

bool CreateInstance();

bool CreateSurface();

bool CreateLogicalDevice();

bool CreateSwapChain();

bool CreateRenderPass();

bool CreateDescriptorSetLayouts();

bool CreateGraphicsPipelines();

bool CreateTextureSamplers();

bool CreateBuffers();

bool CreateDescriptorSets();

#endif //VULKANINTERNAL_H
