//
// Created by NBT22 on 10/20/26.
//

#include <engine/assets/ModelLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanModel.h>
#include <engine/helpers/Macros.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/subsystem/Error.h>
#include <luna/luna.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaCommandBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>

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
static uint32_t mapBottomLevelAccelerationStructureInstanceCount;
static VkAccelerationStructureInstanceKHR *mapBottomLevelAccelerationStructureInstances;
static uint32_t totalBottomLevelAccelerationStructureInstanceCount;

void InitRayTracing()
{
	lunaGetPhysicalDeviceProperties2(device, &physicalDeviceProperties2);

	ListInit(accelerationStructures.vertexScratchBuffers, LIST_UINT64);
	ListInit(accelerationStructures.indexScratchBuffers, LIST_UINT64);
	ListInit(accelerationStructures.bottomLevelAccelerationStructures, LIST_UINT64);
	ListInit(accelerationStructures.bottomLevelBuffers, LIST_UINT64);
	ListInit(accelerationStructures.bottomLevelScratchBuffers, LIST_UINT64);
	ListInit(lodAccelerationStructureDeviceAddresses, LIST_NESTED);

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

void MakeModelBLAS(const ModelDefinition *model,
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

	const uint32_t componentCount = model->materialSlotCount;
	VkAccelerationStructureGeometryKHR *geometries = calloc(componentCount, sizeof(VkAccelerationStructureGeometryKHR));
	CheckAlloc(geometries);
	VkAccelerationStructureBuildRangeInfoKHR *ranges = calloc(componentCount,
															  sizeof(VkAccelerationStructureBuildRangeInfoKHR));
	CheckAlloc(ranges);
	const VkAccelerationStructureBuildRangeInfoKHR **rangePointers = malloc(componentCount * sizeof(*rangePointers));
	CheckAlloc(rangePointers);
	VkAccelerationStructureBuildGeometryInfoKHR
			*buildGeometryInfos = calloc(componentCount, sizeof(VkAccelerationStructureBuildGeometryInfoKHR));
	CheckAlloc(buildGeometryInfos);

	while (lodId >= lodAccelerationStructureDeviceAddresses.length)
	{
		ListAdd(lodAccelerationStructureDeviceAddresses, NULL);
	}
	List *componentAccelerationStructureAddresses = &ListGetNestedList(lodAccelerationStructureDeviceAddresses, lodId);
	ListFree(*componentAccelerationStructureAddresses);
	ListInit(*componentAccelerationStructureAddresses, LIST_UINT64);

	const MaterialSlotVertexData
			*modelFirstMaterialSlotVertexData = ListGetPointer(ListGetNestedList(lodMaterialSlotsVertexData,
																				 model->lods[0].id),
															   0);
	const uint32_t modelFirstIndex = modelFirstMaterialSlotVertexData->firstIndex;
	const int32_t modelVertexOffset = modelFirstMaterialSlotVertexData->vertexOffset;

	const VkDeviceOrHostAddressConstKHR vertexDataAddress = {
		.deviceAddress = lunaGetBufferDeviceAddress(device, *vertexScratchBuffer),
	};
	const VkDeviceOrHostAddressConstKHR indexDataAddress = {
		.deviceAddress = lunaGetBufferDeviceAddress(device, *indexScratchBuffer),
	};

	const List *materialSlotsVertexData = &ListGetNestedList(lodMaterialSlotsVertexData, lodId);
	for (uint32_t i = 0; i < componentCount; i++)
	{
		const MaterialSlotVertexData *materialSlotVertexData = ListGetPointer(*materialSlotsVertexData, i);

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

		const uint32_t triangleCount = materialSlotVertexData->indexCount / 3;
		ranges[i].primitiveCount = triangleCount;
		ranges[i].primitiveOffset = (materialSlotVertexData->firstIndex - modelFirstIndex) * sizeof(uint32_t);
		ranges[i].firstVertex = (uint32_t)(materialSlotVertexData->vertexOffset - modelVertexOffset);
		rangePointers[i] = &ranges[i];

		const VkAccelerationStructureBuildGeometryInfoKHR buildSizesGeometryInfo = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.geometryCount = 1,
			.pGeometries = &geometries[i],
		};
		VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
		};
		vkGetAccelerationStructureBuildSizesKHR(lunaGetVkDevice(device),
												VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
												&buildSizesGeometryInfo,
												&triangleCount,
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
		VkAccelerationStructureKHR *accelerationStructure = ListAdd(accelerationStructures
																			.bottomLevelAccelerationStructures,
																	LUNA_NULL_HANDLE);
		VulkanTest(vkCreateAccelerationStructureKHR(lunaGetVkDevice(device), &createInfo, NULL, accelerationStructure),
				   "Failed to create bottom level acceleration stucture!");

		const VkDeviceOrHostAddressKHR scratchDataAddress = {
			.deviceAddress = lunaGetBufferDeviceAddress(device, *scratchBuffer),
		};
		buildGeometryInfos[i] = buildSizesGeometryInfo;
		buildGeometryInfos[i].dstAccelerationStructure = *accelerationStructure;
		buildGeometryInfos[i].scratchData = scratchDataAddress;

		const VkAccelerationStructureDeviceAddressInfoKHR accelerationStructureDeviceAddressInfo = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.accelerationStructure = *accelerationStructure,
		};
		ListAdd(*componentAccelerationStructureAddresses,
				vkGetAccelerationStructureDeviceAddressKHR(lunaGetVkDevice(device),
														   &accelerationStructureDeviceAddressInfo));
	}

	vkCmdBuildAccelerationStructuresKHR(vkCommandBuffer, componentCount, buildGeometryInfos, rangePointers);

	free(geometries);
	free(ranges);
	free(rangePointers);
	free(buildGeometryInfos);
}

static inline VkAccelerationStructureKHR MakeTrianglesBLAS(const LunaBuffer vertexBuffer,
														   const VkDeviceSize firstVertex,
														   const size_t vertexStride,
														   const LunaBuffer indexBuffer,
														   const VkDeviceSize indexOffset,
														   const uint32_t triangleCount)
{
	const VkDevice vkDevice = lunaGetVkDevice(device);
	const VkDeviceOrHostAddressConstKHR vertexDataAddress = {
		.deviceAddress = lunaGetBufferDeviceAddress(device, vertexBuffer),
	};
	const VkDeviceOrHostAddressConstKHR indexDataAddress = {
		.deviceAddress = lunaGetBufferDeviceAddress(device, indexBuffer) + indexOffset,
	};


	const VkAccelerationStructureGeometryTrianglesDataKHR geometryTrianglesData = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
		.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
		.vertexData = vertexDataAddress,
		.vertexStride = vertexStride,
		.maxVertex = lunaGetBufferSize(vertexBuffer) / vertexStride - 1,
		.indexType = VK_INDEX_TYPE_UINT32,
		.indexData = indexDataAddress,
	};
	const VkAccelerationStructureGeometryDataKHR geometryData = {
		.triangles = geometryTrianglesData,
	};
	const VkAccelerationStructureGeometryKHR geometry = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
		.geometry = geometryData,
		.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
	};

	const VkAccelerationStructureBuildGeometryInfoKHR buildSizesGeometryInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.geometryCount = 1,
		.pGeometries = &geometry,
	};
	VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
	};
	vkGetAccelerationStructureBuildSizesKHR(vkDevice,
											VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
											&buildSizesGeometryInfo,
											&triangleCount,
											&buildSizesInfo);

	LunaBuffer *accelerationStructureBuffer = ListAdd(accelerationStructures.bottomLevelBuffers, LUNA_NULL_HANDLE);
	const LunaBufferCreationInfo bufferCreationInfo = {
		.size = buildSizesInfo.accelerationStructureSize,
		.alignment = 256, // 256 is directly required by the spec
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
		.additionalUsageFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
								VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	};
	VulkanTest(lunaCreateBuffer(device, &bufferCreationInfo, accelerationStructureBuffer),
			   "Failed to create bottom level acceleration structure buffer!");

	VkAccelerationStructureKHR
			*accelerationStructure = ListAdd(accelerationStructures.bottomLevelAccelerationStructures, VK_NULL_HANDLE);
	const VkAccelerationStructureCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.buffer = lunaGetVkBuffer(*accelerationStructureBuffer),
		.offset = lunaGetBufferOffset(*accelerationStructureBuffer),
		.size = lunaGetBufferSize(*accelerationStructureBuffer),
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
	};
	VulkanTest(vkCreateAccelerationStructureKHR(vkDevice, &createInfo, NULL, accelerationStructure),
			   "Failed to create bottom level acceleration structure!");

	LunaBuffer *scratchBuffer = ListAdd(accelerationStructures.bottomLevelScratchBuffers, LUNA_NULL_HANDLE);
	const LunaBufferCreationInfo scratchBufferCreationInfo = {
		.size = buildSizesInfo.buildScratchSize,
		.alignment = physicalDeviceAccelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
		.additionalUsageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	};
	VulkanTest(lunaCreateBuffer(device, &scratchBufferCreationInfo, scratchBuffer),
			   "Failed to create bottom level acceleration structure scratch buffer!");

	const VkDeviceOrHostAddressKHR scratchDataAddress = {
		.deviceAddress = lunaGetBufferDeviceAddress(device, *scratchBuffer),
	};
	const VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.dstAccelerationStructure = *accelerationStructure,
		.geometryCount = 1,
		.pGeometries = &geometry,
		.scratchData = scratchDataAddress,
	};
	const VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo = {
		.primitiveCount = triangleCount,
		.firstVertex = firstVertex,
	};
	const VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfo = &buildRangeInfo;
	vkCmdBuildAccelerationStructuresKHR(vkCommandBuffer, 1, &buildGeometryInfo, &pBuildRangeInfo);

	return *accelerationStructure;
}

// TODO: Investigate using a single BLAS with multiple instances for brushes that are rectangular prisms
void MakeMapModelBLAS(const MapModelsBuffer *buffer,
					  const VkDeviceSize firstVertex,
					  const VkDeviceSize indexOffset,
					  const uint32_t triangleCount,
					  const uint32_t instanceIndex)
{
	const VkAccelerationStructureKHR accelerationStructure = MakeTrianglesBLAS(buffer->vertices,
																			   firstVertex,
																			   sizeof(MapVertex),
																			   buffer->indices,
																			   indexOffset,
																			   triangleCount);

	VkAccelerationStructureInstanceKHR *instance = &mapBottomLevelAccelerationStructureInstances[instanceIndex];
	instance->transform.matrix[0][0] = 1;
	instance->transform.matrix[1][1] = 1;
	instance->transform.matrix[2][2] = 1;
	instance->instanceCustomIndex = (instanceIndex & ((1u << 22) - 1));
	instance->mask = 0xFF;
	const VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
		.accelerationStructure = accelerationStructure,
	};
	instance->accelerationStructureReference = vkGetAccelerationStructureDeviceAddressKHR(lunaGetVkDevice(device),
																						  &deviceAddressInfo);
}

void ClearLunaBufferList(List *list)
{
	for (size_t i = 0; i < list->length; i++)
	{
		lunaDestroyBuffer((LunaBuffer)ListGetUint64(*list, i));
	}
	ListClear(*list);
}

void LoadMapForRaytracing(const Map *map)
{
	ClearLunaBufferList(&accelerationStructures.vertexScratchBuffers);
	ClearLunaBufferList(&accelerationStructures.indexScratchBuffers);
	ClearLunaBufferList(&accelerationStructures.bottomLevelBuffers);
	ClearLunaBufferList(&accelerationStructures.bottomLevelScratchBuffers);
	for (size_t i = 0; i < accelerationStructures.bottomLevelAccelerationStructures.length; i++)
	{
		vkDestroyAccelerationStructureKHR(lunaGetVkDevice(device),
										  (VkAccelerationStructureKHR)
												  ListGetUint64(accelerationStructures
																		.bottomLevelAccelerationStructures,
																i),
										  NULL);
	}
	ListClear(accelerationStructures.bottomLevelAccelerationStructures);

	VulkanTest(lunaResizeBuffer(device, &topLevelAccelerationStructureInstancesBuffer, 0),
			   "Failed to clear top level acceleration structure instances buffer!");

	mapBottomLevelAccelerationStructureInstanceCount = map->modelCount;
	free(mapBottomLevelAccelerationStructureInstances);
	mapBottomLevelAccelerationStructureInstances = calloc(map->modelCount, sizeof(VkAccelerationStructureInstanceKHR));
	CheckAlloc(mapBottomLevelAccelerationStructureInstances);

	LunaBuffer mapBuffers[] = {
		buffers.opaqueMap.vertices,
		buffers.opaqueMap.indices,
		buffers.map.vertices,
		buffers.map.indices,
	};
	const LunaMultiBufferMemoryBarrier multiBufferMemoryBarrier = {
		.sourceStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT | VK_PIPELINE_STAGE_2_COPY_BIT,
		.sourceAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		.destinationAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR |
								 VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
		.bufferCount = ArrayLength(mapBuffers),
		.buffers = mapBuffers,
	};
	const LunaDependencyInfo dependencyInfo = {
		.multiBufferMemoryBarrierCount = 1,
		.multiBufferMemoryBarriers = &multiBufferMemoryBarrier,
	};
	VulkanTest(lunaPipelineBarrier(device, commandBuffer, &dependencyInfo),
			   "Failed to insert pipeline barrier before building map model acceleration structures!");

	VkDeviceSize opaqueVertexOffset = 0;
	VkDeviceSize opaqueIndexOffset = 0;
	VkDeviceSize translucentVertexOffset = 0;
	VkDeviceSize translucentIndexOffset = 0;
	for (size_t i = 0; i < map->modelCount; i++)
	{
		const MapModel *model = map->models + i;
		const bool opaque = LoadImage(model->material->texture)->opaque;
		VkDeviceSize *const vertexOffset = opaque ? &opaqueVertexOffset : &translucentVertexOffset;
		VkDeviceSize *const indexOffset = opaque ? &opaqueIndexOffset : &translucentIndexOffset;
		MakeMapModelBLAS(opaque ? &buffers.opaqueMap : &buffers.map,
						 *vertexOffset,
						 *indexOffset,
						 model->indexCount / 3,
						 i);
		*vertexOffset += model->vertexCount;
		*indexOffset += model->indexCount * sizeof(uint32_t);
	}
}

void UpdateAccelerationStructureInstances(const uint32_t modelInstanceCount,
										  const VkAccelerationStructureInstanceKHR *modelInstances,
										  const uint32_t wallInstanceCount,
										  const VkAccelerationStructureInstanceKHR *wallInstances)
{
	static const VkPipelineStageFlags2 STAGE_MASK = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR |
													VK_PIPELINE_STAGE_2_CLEAR_BIT |
													VK_PIPELINE_STAGE_2_COPY_BIT;
	static const VkAccessFlags2 ACCESS_MASK = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR |
											  VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR |
											  VK_ACCESS_2_TRANSFER_WRITE_BIT;

	totalBottomLevelAccelerationStructureInstanceCount = mapBottomLevelAccelerationStructureInstanceCount +
														 modelInstanceCount +
														 wallInstanceCount;
	const VkDeviceSize mapModelInstancesSize = mapBottomLevelAccelerationStructureInstanceCount *
											   sizeof(VkAccelerationStructureInstanceKHR);
	const VkDeviceSize newSize = totalBottomLevelAccelerationStructureInstanceCount *
								 sizeof(VkAccelerationStructureInstanceKHR);
	if (newSize > lunaGetBufferSize(topLevelAccelerationStructureInstancesBuffer))
	{
		VulkanTest(lunaResizeBuffer(device, &topLevelAccelerationStructureInstancesBuffer, newSize),
				   "Failed to resize top level acceleration structure instances buffer!");
		const LunaBufferWriteInfo instancesBufferWriteInfo = {
			.size = mapModelInstancesSize,
			.data = mapBottomLevelAccelerationStructureInstances,
			.sourceStageMask = STAGE_MASK,
			.sourceAccessMask = ACCESS_MASK,
			.destinationStageMask = STAGE_MASK,
			.destinationAccessMask = ACCESS_MASK,
		};
		VulkanTest(lunaWriteDataToBuffer(device,
										 commandBuffer,
										 topLevelAccelerationStructureInstancesBuffer,
										 &instancesBufferWriteInfo),
				   "Failed to write map model instances to TLAS instance buffer!");
	}
	if (modelInstanceCount > 0 && modelInstances != NULL)
	{
		const LunaBufferWriteInfo instancesBufferWriteInfo = {
			.size = modelInstanceCount * sizeof(VkAccelerationStructureInstanceKHR),
			.data = modelInstances,
			.offset = mapModelInstancesSize,
			.sourceStageMask = STAGE_MASK,
			.sourceAccessMask = ACCESS_MASK,
			.destinationStageMask = STAGE_MASK,
			.destinationAccessMask = ACCESS_MASK,
		};
		VulkanTest(lunaWriteDataToBuffer(device,
										 commandBuffer,
										 topLevelAccelerationStructureInstancesBuffer,
										 &instancesBufferWriteInfo),
				   "Failed to write actor model instances to TLAS instance buffer!");
	}
	if (wallInstanceCount > 0 && wallInstances != NULL)
	{
		const LunaBufferWriteInfo instancesBufferWriteInfo = {
			.size = wallInstanceCount * sizeof(VkAccelerationStructureInstanceKHR),
			.data = wallInstances,
			.offset = mapModelInstancesSize + modelInstanceCount * sizeof(VkAccelerationStructureInstanceKHR),
			.sourceStageMask = STAGE_MASK,
			.sourceAccessMask = ACCESS_MASK,
			.destinationStageMask = STAGE_MASK,
			.destinationAccessMask = ACCESS_MASK,
		};
		VulkanTest(lunaWriteDataToBuffer(device,
										 commandBuffer,
										 topLevelAccelerationStructureInstancesBuffer,
										 &instancesBufferWriteInfo),
				   "Failed to write actor wall instances to TLAS instance buffer!");
	}
}

void BuildTLAS()
{
	vkDestroyAccelerationStructureKHR(lunaGetVkDevice(device),
									  accelerationStructures.topLevelAccelerationStructure,
									  NULL);

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
											&totalBottomLevelAccelerationStructureInstanceCount,
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
		.primitiveCount = totalBottomLevelAccelerationStructureInstanceCount,
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

VkDeviceAddress GetAccelerationStructureDeviceAddress(const ModelDefinition *model, const uint32_t componentIndex)
{
	return (VkDeviceAddress)ListGetUint64(ListGetNestedList(lodAccelerationStructureDeviceAddresses,
															model->lods[LOD_INDEX].id),
										  componentIndex);
}
