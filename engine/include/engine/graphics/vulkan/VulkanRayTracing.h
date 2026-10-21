//
// Created by NBT22 on 10/20/26.
//

#ifndef GAME_VULKANRAYTRACING_H
#define GAME_VULKANRAYTRACING_H

#include <stdint.h>
#include <vulkan/vulkan_core.h>

void InitRayTracing();

void MakeBLAS(const ModelDefinition *modelDefinition,
			  uint32_t vertexBufferSize,
			  const ModelVertex *modelVertices,
			  uint32_t indexBufferSize,
			  const uint32_t *modelIndices);

VkDeviceAddress GetAccelerationStructureDeviceAddress(const ModelDefinition *modelDefinition);

void BuildTLAS(uint32_t instanceCount, const VkAccelerationStructureInstanceKHR *instances);

#endif //GAME_VULKANRAYTRACING_H
