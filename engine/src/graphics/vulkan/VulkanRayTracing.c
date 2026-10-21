//
// Created by NBT22 on 10/20/26.
//

#include <engine/assets/ModelLoader.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanModel.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaCommandBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaTypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include <wchar.h>

#include "luna/luna.h"

// TODO: Split into opaque vs not opaque

typedef struct AccelerationStructures
{
	List vertexScratchBuffers;
	List indexScratchBuffers;

	List bottomLevelAccelerationStructures;
	List bottomLevelBuffers;
	List bottomLevelScratchBuffers;


	VkAccelerationStructureKHR topLevelAccelerationStructure;
	LunaBuffer topLevelBuffer;
	LunaBuffer topLevelScratchBuffer;
} AccelerationStructures;

static const uint32_t LOD_INDEX = 0;

static VkPhysicalDeviceAccelerationStructurePropertiesKHR physicalDeviceAccelerationStructureProperties = {
	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR,
};
static VkPhysicalDeviceProperties2 physicalDeviceProperties2 = {
	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
	.pNext = &physicalDeviceAccelerationStructureProperties,
};
static AccelerationStructures accelerationStructures;
static List lodAccelerationStructureDeviceAddresses;
static LunaBuffer topLevelAccelerationStructureInstancesBuffer;

void InitRayTracing()
{
	lunaGetPhysicalDeviceProperties2(device, &physicalDeviceProperties2);

	ListInit(accelerationStructures.vertexScratchBuffers, LIST_UINT64);
	ListInit(accelerationStructures.indexScratchBuffers, LIST_UINT64);
	ListInit(accelerationStructures.bottomLevelAccelerationStructures, LIST_UINT64);
	ListInit(accelerationStructures.bottomLevelBuffers, LIST_UINT64);
	ListInit(accelerationStructures.bottomLevelScratchBuffers, LIST_UINT64);
	ListInit(lodAccelerationStructureDeviceAddresses, LIST_UINT64);

	const LunaBufferCreationInfo topLevelBufferCreationInfo = {
		.alignment = 256, // 256 is directly required by the spec
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
		.additionalUsageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
								VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
	};
	VulkanTest(lunaCreateBuffer(device, &topLevelBufferCreationInfo, &accelerationStructures.topLevelBuffer),
			   "Failed to create top level acceleration structure buffer!");

	const LunaBufferCreationInfo topLevelScratchBufferCreationInfo = {
		.alignment = physicalDeviceAccelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
		.additionalUsageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	};
	VulkanTest(lunaCreateBuffer(device,
								&topLevelScratchBufferCreationInfo,
								&accelerationStructures.topLevelScratchBuffer),
			   "Failed to create top level acceleration structure scratch buffer!");

	const LunaBufferCreationInfo topLevelAccelerationStructureInstancesBufferCreationInfo = {
		.alignment = 16, // 16 is directly required by the spec
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
		.additionalUsageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
								VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
	};
	VulkanTest(lunaCreateBuffer(device,
								&topLevelAccelerationStructureInstancesBufferCreationInfo,
								&topLevelAccelerationStructureInstancesBuffer),
			   "Failed to create top level acceleration structure instances buffer!");
}

void MakeBLAS(const ModelDefinition *model,
			  const uint32_t vertexBufferSize,
			  const ModelVertex *modelVertices,
			  const uint32_t indexBufferSize,
			  const uint32_t *modelIndices)
{
	const uint32_t lodId = model->lods[LOD_INDEX].id;

	LunaBuffer *vertexScratchBuffer = ListAdd(accelerationStructures.vertexScratchBuffers, LUNA_NULL_HANDLE);
	const LunaBufferCreationInfo vertexScratchBufferCreationInfo = {
		.size = vertexBufferSize,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
		.additionalUsageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
								VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
	};
	VulkanTest(lunaCreateBuffer(device, &vertexScratchBufferCreationInfo, vertexScratchBuffer),
			   "Failed to create BLAS vertex scratch buffer!");
	const LunaBufferWriteInfo vertexScratchBufferWriteInfo = {
		.size = vertexBufferSize,
		.data = modelVertices,
		.sourceStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT | VK_PIPELINE_STAGE_2_COPY_BIT,
		.sourceAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		.destinationAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,
	};
	VulkanTest(lunaWriteDataToBuffer(device, commandBuffer, *vertexScratchBuffer, &vertexScratchBufferWriteInfo),
			   "Failed to write vertex data to scratch buffer!");

	LunaBuffer *indexScratchBuffer = ListAdd(accelerationStructures.indexScratchBuffers, LUNA_NULL_HANDLE);
	const LunaBufferCreationInfo indexScratchBufferCreationInfo = {
		.size = indexBufferSize,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
		.additionalUsageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
								VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
	};
	VulkanTest(lunaCreateBuffer(device, &indexScratchBufferCreationInfo, indexScratchBuffer),
			   "Failed to create BLAS index scratch buffer!");
	const LunaBufferWriteInfo indexScratchBufferWriteInfo = {
		.size = indexBufferSize,
		.data = modelIndices,
		.sourceStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT | VK_PIPELINE_STAGE_2_COPY_BIT,
		.sourceAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		.destinationAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,
	};
	VulkanTest(lunaWriteDataToBuffer(device, commandBuffer, *indexScratchBuffer, &indexScratchBufferWriteInfo),
			   "Failed to write index data to scratch buffer!");

	VkAccelerationStructureGeometryKHR *geometries = calloc(model->materialSlotCount,
															sizeof(VkAccelerationStructureGeometryKHR));
	CheckAlloc(geometries);
	uint32_t *triangleCounts = malloc(model->materialSlotCount * sizeof(uint32_t));
	CheckAlloc(triangleCounts);
	VkAccelerationStructureBuildRangeInfoKHR *ranges = calloc(model->materialSlotCount,
															  sizeof(VkAccelerationStructureBuildRangeInfoKHR));
	CheckAlloc(ranges);

	List *materialSlotsVertexData = &ListGetNestedList(lodMaterialSlotsVertexData, lodId);
	for (uint32_t i = 0; i < model->materialSlotCount; i++)
	{
		const MaterialSlotVertexData *materialSlotVertexData = ListGetPointer(*materialSlotsVertexData, i);

		const VkDeviceOrHostAddressConstKHR vertexDataAddress = {
			.deviceAddress = lunaGetBufferDeviceAddress(device, *vertexScratchBuffer),
		};
		const VkDeviceOrHostAddressConstKHR indexDataAddress = {
			.deviceAddress = lunaGetBufferDeviceAddress(device, *indexScratchBuffer),
		};
		const VkAccelerationStructureGeometryTrianglesDataKHR geometryTrianglesData = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
			.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
			.vertexData = vertexDataAddress,
			.vertexStride = sizeof(ModelVertex),
			.maxVertex = vertexBufferSize / sizeof(ModelVertex) - 1,
			.indexType = VK_INDEX_TYPE_UINT32,
			.indexData = indexDataAddress,
		};
		const VkAccelerationStructureGeometryDataKHR geometryData = {
			.triangles = geometryTrianglesData,
		};
		geometries[i].sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometries[i].geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometries[i].geometry = geometryData;
		geometries[i].flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		triangleCounts[i] = materialSlotVertexData->indexCount / 3;
		ranges[i].primitiveCount = triangleCounts[i];
		// ranges[i].firstVertex = materialSlotVertexData->vertexOffset;
	}

	const VkAccelerationStructureBuildGeometryInfoKHR buildSizesGeometryInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.geometryCount = model->materialSlotCount,
		.pGeometries = geometries,
	};
	VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
	};
	vkGetAccelerationStructureBuildSizesKHR(lunaGetVkDevice(device),
											VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
											&buildSizesGeometryInfo,
											triangleCounts,
											&buildSizesInfo);

	const LunaBufferCreationInfo bufferCreationInfo = {
		.size = buildSizesInfo.accelerationStructureSize,
		.alignment = 256, // 256 is directly required by the spec
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
		.additionalUsageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
								VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
	};
	LunaBuffer *buffer = ListAdd(accelerationStructures.bottomLevelBuffers, LUNA_NULL_HANDLE);
	VulkanTest(lunaCreateBuffer(device, &bufferCreationInfo, buffer),
			   "Failed to create buffer for bottom level acceleration structure!");

	const LunaBufferCreationInfo scratchBufferCreationInfo = {
		.size = buildSizesInfo.buildScratchSize,
		.alignment = physicalDeviceAccelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
		.additionalUsageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	};
	LunaBuffer *scratchBuffer = ListAdd(accelerationStructures.bottomLevelScratchBuffers, LUNA_NULL_HANDLE);
	VulkanTest(lunaCreateBuffer(device, &scratchBufferCreationInfo, scratchBuffer),
			   "Failed to create bottom level acceleration structure scratch buffer!");

	const VkAccelerationStructureCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.buffer = lunaGetVkBuffer(*buffer),
		.offset = lunaGetBufferOffset(*buffer),
		.size = lunaGetBufferSize(*buffer),
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
	};
	VkAccelerationStructureKHR *
			accelerationStructure = ListAdd(accelerationStructures.bottomLevelAccelerationStructures, LUNA_NULL_HANDLE);
	VulkanTest(vkCreateAccelerationStructureKHR(lunaGetVkDevice(device), &createInfo, NULL, accelerationStructure),
			   "Failed to create bottom level acceleration stucture!");

	const VkDeviceOrHostAddressKHR scratchDataAddress = {
		.deviceAddress = lunaGetBufferDeviceAddress(device, *scratchBuffer),
	};
	const VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.dstAccelerationStructure = *accelerationStructure,
		.geometryCount = model->materialSlotCount,
		.pGeometries = geometries,
		.scratchData = scratchDataAddress,
	};
	vkCmdBuildAccelerationStructuresKHR(lunaGetVkCommandBuffer(commandBuffer),
										1,
										&buildGeometryInfo,
										(const VkAccelerationStructureBuildRangeInfoKHR *const *)&ranges);

	while (lodId >= lodAccelerationStructureDeviceAddresses.length)
	{
		ListAdd(lodAccelerationStructureDeviceAddresses, VK_NULL_HANDLE);
	}
	VkDeviceAddress *accelerationStructureAddress = &ListGetUint64(lodAccelerationStructureDeviceAddresses, lodId);
	const VkAccelerationStructureDeviceAddressInfoKHR accelerationStructureDeviceAddressInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
		.accelerationStructure = *accelerationStructure,
	};
	*accelerationStructureAddress = vkGetAccelerationStructureDeviceAddressKHR(lunaGetVkDevice(device),
																			   &accelerationStructureDeviceAddressInfo);

	free(geometries);
	free(triangleCounts);
	free(ranges);
}

VkDeviceAddress GetAccelerationStructureDeviceAddress(const ModelDefinition *model)
{
	return (VkDeviceAddress)ListGetUint64(lodAccelerationStructureDeviceAddresses, model->lods[LOD_INDEX].id);
}

void BuildTLAS(const uint32_t instanceCount, const VkAccelerationStructureInstanceKHR *instances)
{
	VulkanTest(lunaResizeBuffer(device,
								&topLevelAccelerationStructureInstancesBuffer,
								instanceCount * sizeof(*instances)),
			   "Failed to resize top level acceleration structure instances buffer!");
	const LunaBufferWriteInfo instancesBufferWriteInfo = {
		.size = instanceCount * sizeof(*instances),
		.data = instances,
	};
	VulkanTest(lunaWriteDataToBuffer(device,
									 commandBuffer,
									 topLevelAccelerationStructureInstancesBuffer,
									 &instancesBufferWriteInfo),
			   "Failed to write TLAS instance buffer!");

	const VkDeviceOrHostAddressConstKHR instancesDataAddress = {
		.deviceAddress = lunaGetBufferDeviceAddress(device, topLevelAccelerationStructureInstancesBuffer),
	};
	const VkAccelerationStructureGeometryInstancesDataKHR geometryInstancesData = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
		.arrayOfPointers = VK_FALSE,
		.data = instancesDataAddress,
	};
	const VkAccelerationStructureGeometryDataKHR geometryData = {
		.instances = geometryInstancesData,
	};
	const VkAccelerationStructureGeometryKHR geometry = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
		.geometry = geometryData,
	};

	const VkAccelerationStructureBuildGeometryInfoKHR buildSizesGeometryInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.geometryCount = 1,
		.pGeometries = &geometry,
	};
	VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
	};
	vkGetAccelerationStructureBuildSizesKHR(lunaGetVkDevice(device),
											VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
											&buildSizesGeometryInfo,
											&instanceCount,
											&buildSizesInfo);

	VulkanTest(lunaResizeBuffer(device,
								&accelerationStructures.topLevelBuffer,
								buildSizesInfo.accelerationStructureSize),
			   "Failed to resize top level acceleration structure buffer!");
	VulkanTest(lunaResizeBuffer(device, &accelerationStructures.topLevelScratchBuffer, buildSizesInfo.buildScratchSize),
			   "Failed to resize top level acceleration structure scratch buffer!");

	const VkAccelerationStructureCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.buffer = lunaGetVkBuffer(accelerationStructures.topLevelBuffer),
		.offset = lunaGetBufferOffset(accelerationStructures.topLevelBuffer),
		.size = lunaGetBufferSize(accelerationStructures.topLevelBuffer),
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
	};
	VulkanTest(vkCreateAccelerationStructureKHR(lunaGetVkDevice(device),
												&createInfo,
												NULL,
												&accelerationStructures.topLevelAccelerationStructure),
			   "Failed to create top level acceleration structure!");

	const VkDeviceOrHostAddressKHR scratchDataAddress = {
		.deviceAddress = lunaGetBufferDeviceAddress(device, accelerationStructures.topLevelScratchBuffer),
	};
	const VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.dstAccelerationStructure = accelerationStructures.topLevelAccelerationStructure,
		.geometryCount = 1,
		.pGeometries = &geometry,
		.scratchData = scratchDataAddress,
	};
	const VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo = {
		.primitiveCount = instanceCount,
	};
	const VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfo = &buildRangeInfo;
	vkCmdBuildAccelerationStructuresKHR(lunaGetVkCommandBuffer(commandBuffer), 1, &buildGeometryInfo, &pBuildRangeInfo);


	const VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureWriteInfo = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
		.accelerationStructureCount = 1,
		.pAccelerationStructures = &accelerationStructures.topLevelAccelerationStructure,
	};
	const LunaWriteDescriptorSet descriptorWrite = {
		.pNext = &accelerationStructureWriteInfo,
		.descriptorSet = descriptorSets.rayTracing.set,
		.bindingName = "Acceleration Structure",
		.descriptorCount = 1,
	};
	lunaWriteDescriptorSets(device, 1, &descriptorWrite);
}
