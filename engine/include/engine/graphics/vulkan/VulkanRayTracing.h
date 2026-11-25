//
// Created by NBT22 on 10/20/26.
//

#ifndef GAME_VULKANRAYTRACING_H
#define GAME_VULKANRAYTRACING_H

#include <stdint.h>
#include <vulkan/vulkan_core.h>

void InitRayTracing();

void MakeModelBLAS(const ModelDefinition *modelDefinition,
				   uint32_t vertexBufferSize,
				   const ModelVertex *modelVertices,
				   uint32_t indexBufferSize,
				   const uint32_t *modelIndices);

void LoadMapForRaytracing(const Map *map);

void UpdateAccelerationStructureInstances(uint32_t modelInstanceCount,
										  const VkAccelerationStructureInstanceKHR *modelInstances,
										  uint32_t wallInstanceCount,
										  const VkAccelerationStructureInstanceKHR *wallInstances);

void BuildTLAS();

VkDeviceAddress GetAccelerationStructureDeviceAddress(const ModelDefinition *modelDefinition, uint32_t componentIndex);

#endif //GAME_VULKANRAYTRACING_H
