// Created by NBT22 on 1/23/26.
//

#include <assert.h>
#include <engine/assets/ModelLoader.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/graphics/vulkan/VulkanActors.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/RVec3.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <luna/luna.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
	uint32_t indexCount;
	uint32_t firstIndex;
	int32_t vertexOffset;
} MaterialSlotVertexData;

typedef struct
{
	/// A pointer to the VkDrawIndexedIndirectCommand structure used for drawing this material as shaded
	VkDrawIndexedIndirectCommand *shadedDrawInfo;
	/// A pointer to the VkDrawIndexedIndirectCommand structure used for drawing this material as unshaded
	VkDrawIndexedIndirectCommand *unshadedDrawInfo;
} MaterialSlotData;

typedef struct
{
	/// The number of instances
	uint32_t instanceCount;
	/// A list of @c MaterialSlotData structures, indexed using a material slot index
	List materialSlots;
} LodMaterialSlotsData;

typedef struct
{
	List *lodInstanceCounts;
	uint32_t modelInstanceCount;
	uint32_t shadedModelInstanceCount;
	uint32_t unshadedModelInstanceCount;
	uint32_t modelDrawCount;
	uint32_t shadedWallsInstanceCount;
	uint32_t unshadedWallsInstanceCount;
	bool shouldReallocModels;
	bool shouldReallocShadedWalls;
	bool shouldReallocUnshadedWalls;
} InstanceDataReallocInfo;

static uint32_t allocatedBufferCount;
static size_t bufferVertexCount;
static size_t bufferIndexCount;
static uint32_t *shadedModelsInstanceIndices;
static uint32_t *unshadedModelsInstanceIndices;
static ActorModelInstanceData *modelsInstanceData;
static VkDrawIndexedIndirectCommand *shadedModelsDrawInfo;
static VkDrawIndexedIndirectCommand *unshadedModelsDrawInfo;
static ModelActorCullingInfo *shadedModelsCullingInfo;
static ModelActorCullingInfo *unshadedModelsCullingInfo;
static ActorWallInstanceData *wallsInstanceData;
static CullingInfo *shadedWallsCullingInfo;
static CullingInfo *unshadedWallsCullingInfo;

/// A list of uint32_t model ids that are currently loaded
static List loadedModelIds;
/// A list, indexed with a lod id, that contains lists of @c MaterialSlotVertexData structures for each material slot
static List lodMaterialSlotsVertexData;
/// A list of @c LodMaterialSlotsData structures, indexed using a lod id
static List lodMaterialSlotsData;

static inline void ClearLodMaterialSlotsData()
{
	for (size_t i = 0; i < lodMaterialSlotsData.length; i++)
	{
		LodMaterialSlotsData *materialSlotData = ListGetPointer(lodMaterialSlotsData, i);
		ListAndContentsFree(materialSlotData->materialSlots);
	}
	ListAndContentsFree(lodMaterialSlotsData);
}

void InitActorLoadingVariables()
{
	ListInit(loadedModelIds, LIST_UINT32);
	ListInit(lodMaterialSlotsVertexData, LIST_NESTED);
	ListInit(lodMaterialSlotsData, LIST_POINTER);
}

static inline VkResult LoadModelLods(const ModelDefinition *model)
{
	if (ListFind(loadedModelIds, model->id) != SIZE_MAX)
	{
		// Model is already loaded, so we're done here
		return VK_SUCCESS;
	}
	ListAdd(loadedModelIds, model->id);

	int32_t vertexCount = 0;
	uint32_t indexCount = 0;
	for (uint32_t i = 0; i < model->lodCount; i++)
	{
		const uint32_t lodId = model->lods[i].id;
		while (lodId >= lodMaterialSlotsVertexData.length)
		{
			ListAdd(lodMaterialSlotsVertexData, NULL);
		}
		List *materialSlotsVertexData = &ListGetNestedList(lodMaterialSlotsVertexData, lodId);
		assert(materialSlotsVertexData->length == 0);
		for (uint32_t j = 0; j < model->materialSlotCount; j++)
		{
			MaterialSlotVertexData *materialSlotVertexData = malloc(sizeof(MaterialSlotVertexData));
			CheckAlloc(materialSlotVertexData);
			materialSlotVertexData->indexCount = model->lods[i].components[j].indexCount;
			materialSlotVertexData->firstIndex = bufferIndexCount + indexCount;
			materialSlotVertexData->vertexOffset = (int32_t)(bufferVertexCount + vertexCount);
			ListAdd(*materialSlotsVertexData, materialSlotVertexData);

			indexCount += materialSlotVertexData->indexCount;
		}

		vertexCount += (int32_t)model->lods[i].vertexCount;
	}
	if (vertexCount == 0 || indexCount == 0)
	{
		assert(vertexCount == 0 && indexCount == 0);
		return VK_SUCCESS;
	}

	const size_t verticesSize = vertexCount * sizeof(ModelVertex);
	const size_t indicesSize = indexCount * sizeof(uint32_t);

	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffers.actorModels.vertices,
											lunaGetBufferSize(buffers.actorModels.vertices) + verticesSize),
						   "Failed to resize actor model vertex buffer!");
	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffers.actorModels.indices,
											lunaGetBufferSize(buffers.actorModels.indices) + indicesSize),
						   "Failed to resize actor model index buffer!");

	ModelVertex *vertexData = malloc(verticesSize);
	uint32_t *indexData = malloc(indicesSize);

	size_t vertexOffset = 0;
	size_t indexOffset = 0;
	for (uint32_t i = 0; i < model->lodCount; i++)
	{
		const ModelLod *lod = &model->lods[i];
		memcpy(vertexData + vertexOffset, lod->vertexData, lod->vertexCount * sizeof(ModelVertex));
		vertexOffset += lod->vertexCount;
		for (uint32_t materialSlotIndex = 0; materialSlotIndex < model->materialSlotCount; materialSlotIndex++)
		{
			const ModelComponent *component = &lod->components[materialSlotIndex];
			memcpy(indexData + indexOffset, component->indices, component->indexCount * sizeof(uint32_t));
			indexOffset += component->indexCount;
		}
	}

	const LunaBufferWriteInfo vertexBufferWriteInfo = {
		.bytes = verticesSize,
		.data = vertexData,
		.offset = bufferVertexCount * sizeof(ModelVertex),
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorModels.vertices,
												 &vertexBufferWriteInfo),
						   "Failed to write model vertex data to buffer!");
	const LunaBufferWriteInfo indexBufferWriteInfo = {
		.bytes = indicesSize,
		.data = indexData,
		.offset = bufferIndexCount * sizeof(uint32_t),
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorModels.indices,
												 &indexBufferWriteInfo),
						   "Failed to write model index data to buffer!");

	bufferVertexCount += vertexCount;
	bufferIndexCount += indexCount;

	free(vertexData);
	free(indexData);

	return VK_SUCCESS;
}

VkResult LoadActors(const LockingList *actors)
{
	buffers.actorWalls.shadedInstanceCount = 0;
	buffers.actorWalls.unshadedInstanceCount = 0;

	ListLock(*actors);
	for (size_t i = 0; i < actors->length; i++)
	{
		const Actor *actor = ListGetPointer(*actors, i);
		if (actor->hasModel)
		{
			VulkanTestReturnResult(LoadModelLods(actor->model), "Failed to load actor!");
		}
	}
	ListUnlock(*actors);

	return VK_SUCCESS;
}

static inline bool ShouldReallocInstanceData(const LockingList *actors, InstanceDataReallocInfo *reallocInfo)
{
	ListInit(*reallocInfo->lodInstanceCounts, LIST_UINT32);
	for (size_t i = 0; i < actors->length; i++)
	{
		const Actor *actor = ListGetPointer(*actors, i);
		if (!actor->visible)
		{
			continue;
		}
		if (actor->hasModel)
		{
			reallocInfo->modelInstanceCount += actor->model->materialSlotCount;
			const uint32_t lodId = actor->model->lods[actor->currentLod].id;
			while (lodId >= reallocInfo->lodInstanceCounts->length)
			{
				ListAdd(*reallocInfo->lodInstanceCounts, 0);
			}
			const uint32_t currentInstanceCount = ListGetUint32(*reallocInfo->lodInstanceCounts, lodId);
			if (currentInstanceCount == 0)
			{
				reallocInfo->modelDrawCount += actor->model->materialSlotCount;
				if (lodId < lodMaterialSlotsData.length)
				{
					const LodMaterialSlotsData *materialSlotDatas = ListGetPointer(lodMaterialSlotsData, lodId);
					for (uint32_t j = 0; j < materialSlotDatas->materialSlots.length; j++)
					{
						const MaterialSlotData *materialSlotData = ListGetPointer(materialSlotDatas->materialSlots, j);
						materialSlotData->shadedDrawInfo->instanceCount = 0;
						materialSlotData->unshadedDrawInfo->instanceCount = 0;
						materialSlotData->unshadedDrawInfo->firstInstance = materialSlotData->shadedDrawInfo
																					->firstInstance;
					}
				}
			}
			ListSet(*reallocInfo->lodInstanceCounts, lodId, currentInstanceCount + 1);
		} else if (actor->wall)
		{
			if (actor->wall->unshaded)
			{
				reallocInfo->unshadedWallsInstanceCount++;
			} else
			{
				reallocInfo->shadedWallsInstanceCount++;
			}
		}
	}

	reallocInfo->shouldReallocShadedWalls = reallocInfo->shadedWallsInstanceCount !=
											buffers.actorWalls.shadedInstanceCount;
	reallocInfo->shouldReallocUnshadedWalls = reallocInfo->unshadedWallsInstanceCount !=
											  buffers.actorWalls.unshadedInstanceCount;

	if (frustumCount != allocatedBufferCount)
	{
		reallocInfo->shouldReallocModels = true;
		reallocInfo->shouldReallocShadedWalls = true;
		reallocInfo->shouldReallocUnshadedWalls = true;
		return true;
	}

	if (reallocInfo->lodInstanceCounts->length != lodMaterialSlotsData.length)
	{
		reallocInfo->shouldReallocModels = true;
		return true;
	}

	for (uint32_t i = 0; i < reallocInfo->lodInstanceCounts->length; i++)
	{
		const uint32_t instanceCount = ListGetUint32(*reallocInfo->lodInstanceCounts, i);
		const LodMaterialSlotsData *materialSlotDatas = ListGetPointer(lodMaterialSlotsData, i);
		if (instanceCount != materialSlotDatas->instanceCount)
		{
			reallocInfo->shouldReallocModels = true;
			return true;
		}
	}

	return reallocInfo->shouldReallocShadedWalls || reallocInfo->shouldReallocUnshadedWalls;
}

static inline VkResult ReallocateInstanceData(const LockingList *actors, const InstanceDataReallocInfo *reallocInfo)
{
	const uint32_t instanceIndicesBytes = reallocInfo->modelInstanceCount * sizeof(uint32_t);
	const uint32_t drawInfoBytes = reallocInfo->modelDrawCount * sizeof(VkDrawIndexedIndirectCommand);
	const uint32_t cullingInfoBytes = sizeof(uint32_t) +
									  reallocInfo->modelInstanceCount * sizeof(ModelActorCullingInfo);

	if (reallocInfo->shouldReallocModels)
	{
		free(shadedModelsInstanceIndices);
		free(unshadedModelsInstanceIndices);
		free(modelsInstanceData);
		free(shadedModelsDrawInfo);
		free(unshadedModelsDrawInfo);
		free(shadedModelsCullingInfo);
		free(unshadedModelsCullingInfo);

		shadedModelsInstanceIndices = malloc(instanceIndicesBytes);
		CheckAlloc(shadedModelsInstanceIndices);
		unshadedModelsInstanceIndices = malloc(instanceIndicesBytes);
		CheckAlloc(unshadedModelsInstanceIndices);
		modelsInstanceData = malloc(reallocInfo->modelInstanceCount * sizeof(ActorModelInstanceData));
		CheckAlloc(modelsInstanceData);
		shadedModelsDrawInfo = calloc(1, drawInfoBytes);
		CheckAlloc(shadedModelsDrawInfo);
		unshadedModelsDrawInfo = calloc(1, drawInfoBytes);
		CheckAlloc(unshadedModelsDrawInfo);
		shadedModelsCullingInfo = malloc(cullingInfoBytes);
		CheckAlloc(shadedModelsCullingInfo);
		unshadedModelsCullingInfo = malloc(cullingInfoBytes);
		CheckAlloc(unshadedModelsCullingInfo);

		ClearLodMaterialSlotsData();
		ListInit(lodMaterialSlotsData, LIST_POINTER);
		for (size_t i = 0; i < reallocInfo->lodInstanceCounts->length; i++)
		{
			LodMaterialSlotsData *materialSlotsData = malloc(sizeof(LodMaterialSlotsData));
			materialSlotsData->instanceCount = ListGetUint32(*reallocInfo->lodInstanceCounts, i);
			ListInit(materialSlotsData->materialSlots, LIST_POINTER);
			ListAdd(lodMaterialSlotsData, materialSlotsData);
		}
	}

	size_t instanceDataOffset = 0;
	size_t drawInfoOffset = 0;
	for (size_t i = 0; i < actors->length; i++)
	{
		const Actor *actor = ListGetPointer(*actors, i);
		if (!actor->visible)
		{
			continue;
		}
		if (actor->hasModel && reallocInfo->shouldReallocModels)
		{
			const uint32_t lodId = actor->model->lods[actor->currentLod].id;
			if (ListFind(loadedModelIds, actor->model->id) == SIZE_MAX)
			{
				VulkanTestReturnResult(LoadModelLods(actor->model), "Failed to load new model lods!");
			}
			LodMaterialSlotsData *materialSlotsData = ListGetPointer(lodMaterialSlotsData, lodId);
			if (materialSlotsData->materialSlots.length == 0)
			{
				List *materialSlotsVertexData = &ListGetNestedList(lodMaterialSlotsVertexData, lodId);
				for (uint32_t j = 0; j < actor->model->materialSlotCount; j++)
				{
					const MaterialSlotVertexData *materialSlotVertexData = ListGetPointer(*materialSlotsVertexData, j);
					MaterialSlotData *materialSlotData = malloc(sizeof(MaterialSlotData));
					materialSlotData->shadedDrawInfo = shadedModelsDrawInfo + drawInfoOffset;
					materialSlotData->shadedDrawInfo->indexCount = materialSlotVertexData->indexCount;
					materialSlotData->shadedDrawInfo->instanceCount = 0;
					materialSlotData->shadedDrawInfo->firstIndex = materialSlotVertexData->firstIndex;
					materialSlotData->shadedDrawInfo->vertexOffset = materialSlotVertexData->vertexOffset;
					materialSlotData->shadedDrawInfo->firstInstance = instanceDataOffset;
					materialSlotData->unshadedDrawInfo = unshadedModelsDrawInfo + drawInfoOffset;
					*materialSlotData->unshadedDrawInfo = *materialSlotData->shadedDrawInfo;
					ListAdd(materialSlotsData->materialSlots, materialSlotData);
					drawInfoOffset++;
					instanceDataOffset += materialSlotsData->instanceCount;
				}
			} else
			{
				for (uint32_t j = 0; j < actor->model->materialSlotCount; j++)
				{
					MaterialSlotData *materialSlotData = ListGetPointer(materialSlotsData->materialSlots, j);
					materialSlotData->shadedDrawInfo->instanceCount = 0;
					materialSlotData->unshadedDrawInfo->instanceCount = 0;
					materialSlotData->unshadedDrawInfo->firstInstance = materialSlotData->shadedDrawInfo->firstInstance;
				}
			}
		}
	}

	if (reallocInfo->shouldReallocModels)
	{
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorModels.shadedCullingInfo,
												cullingInfoBytes),
							   "Failed to resize actor models shaded culling info buffer!");
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorModels.unshadedCullingInfo,
												cullingInfoBytes),
							   "Failed to resize actor models unshaded culling info buffer!");
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorModels.shadedUnculledInstanceIndices,
												instanceIndicesBytes),
							   "Failed to resize actor models unculled shaded instance indices buffer!");
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorModels.unshadedUnculledInstanceIndices,
												instanceIndicesBytes),
							   "Failed to resize actor models unculled unshaded instance indices buffer!");
		for (uint32_t i = 0; i < frustumCount; i++)
		{
			LunaBuffer *shadedInstanceIndices = (LunaBuffer *)&ListGetPointer(buffers.actorModels.shadedInstanceIndices,
																			  i);
			LunaBuffer *unshadedInstanceIndices = (LunaBuffer *)&ListGetPointer(buffers.actorModels
																						.unshadedInstanceIndices,
																				i);
			LunaBuffer *shadedDrawInfo = (LunaBuffer *)&ListGetPointer(buffers.actorModels.shadedDrawInfo, i);
			LunaBuffer *unshadedDrawInfo = (LunaBuffer *)&ListGetPointer(buffers.actorModels.unshadedDrawInfo, i);

			VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, shadedInstanceIndices, instanceIndicesBytes),
								   "Failed to resize actor models shaded instance indices buffer!");
			VulkanTestReturnResult(lunaResizeBuffer(device,
													commandBuffer,
													unshadedInstanceIndices,
													instanceIndicesBytes),
								   "Failed to resize actor models unshaded instance indices buffer!");
			VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, shadedDrawInfo, drawInfoBytes),
								   "Failed to resize actor models shaded draw info buffer!");
			VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, unshadedDrawInfo, drawInfoBytes),
								   "Failed to resize actor models unshaded draw info buffer!");

			const uint32_t instanceIndicesShadedIndex = PER_FRUSTUM_BUFFER_COUNT * i +
														PER_FRUSTUM_BUFFER_MODEL_ACTOR_INSTANCE_INDICES_OFFSET;
			const uint32_t drawInfoShadedIndex = PER_FRUSTUM_BUFFER_COUNT * i +
												 PER_FRUSTUM_BUFFER_MODEL_ACTOR_DRAW_INFO_OFFSET;
			ListSet(perFrustumBuffersHandles, instanceIndicesShadedIndex, *shadedInstanceIndices);
			ListSet(perFrustumBuffersHandles, instanceIndicesShadedIndex + 1, *unshadedInstanceIndices);
			ListSet(perFrustumBuffersHandles, drawInfoShadedIndex, *shadedDrawInfo);
			ListSet(perFrustumBuffersHandles, drawInfoShadedIndex + 1, *unshadedDrawInfo);

			FrustumCullingData *frustum = &frustums[i];
			frustum->shadedActorModelsCullingInfo = lunaGetBufferDeviceAddress(device,
																			   buffers.actorModels.shadedCullingInfo);
			frustum->shadedActorModelsUnculledInstanceIndices = lunaGetBufferDeviceAddress(
					device,
					buffers.actorModels.shadedUnculledInstanceIndices);
			frustum->shadedActorModelsInstanceIndices = lunaGetBufferDeviceAddress(device, *shadedInstanceIndices);
			frustum->shadedActorModelsDrawInfo = lunaGetBufferDeviceAddress(device, *shadedDrawInfo);

			frustum->unshadedActorModelsCullingInfo = lunaGetBufferDeviceAddress(device,
																				 buffers.actorModels
																						 .unshadedCullingInfo);
			frustum->unshadedActorModelsUnculledInstanceIndices = lunaGetBufferDeviceAddress(
					device,
					buffers.actorModels.unshadedUnculledInstanceIndices);
			frustum->unshadedActorModelsInstanceIndices = lunaGetBufferDeviceAddress(device, *unshadedInstanceIndices);
			frustum->unshadedActorModelsDrawInfo = lunaGetBufferDeviceAddress(device, *unshadedDrawInfo);
		}

		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorModels.instanceData,
												reallocInfo->modelInstanceCount * sizeof(ActorModelInstanceData)),
							   "Failed to resize actor models instance data buffer!");
		const LunaDescriptorBufferInfo instanceDataBuffer = {
			.buffer = buffers.actorModels.instanceData,
		};
		const LunaWriteDescriptorSet instanceDataWrite = {
			.descriptorSet = descriptorSets.common.set,
			.bindingName = "Actor Instance Data",
			.descriptorArrayElement = 1,
			.descriptorCount = 1,
			.bufferInfos = &instanceDataBuffer,
		};
		lunaWriteDescriptorSets(device, 1, &instanceDataWrite);
	}

	if (reallocInfo->shouldReallocShadedWalls)
	{
		buffers.actorWalls.shadedInstanceCount = reallocInfo->shadedWallsInstanceCount;

		const size_t cullingInfoSize = sizeof(uint32_t) + sizeof(CullingInfo) * buffers.actorWalls.shadedInstanceCount;
		free(shadedWallsCullingInfo);
		shadedWallsCullingInfo = malloc(cullingInfoSize);
		CheckAlloc(shadedWallsCullingInfo);
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorWalls.shadedCullingInfo,
												cullingInfoSize),
							   "Failed to resize shaded actor walls culling info buffer!");
		VulkanTestReturnResult(lunaWriteUintToBuffer(device,
													 commandBuffer,
													 buffers.actorWalls.shadedCullingInfo,
													 0,
													 buffers.actorWalls.shadedInstanceCount,
													 NULL),
							   "Failed to write shaded actor walls culling info to buffer!");

		for (uint32_t i = 0; i < frustumCount; i++)
		{
			LunaBuffer *buffer = (LunaBuffer *)&ListGetPointer(buffers.actorWalls.shadedInstanceIndices, i);
			VulkanTestReturnResult(lunaResizeBuffer(device,
													commandBuffer,
													buffer,
													sizeof(uint32_t) * buffers.actorWalls.shadedInstanceCount),
								   "Failed to resize shaded actor walls instance indices buffer!");
			const uint32_t index = PER_FRUSTUM_BUFFER_COUNT * i + PER_FRUSTUM_BUFFER_WALL_ACTOR_INSTANCE_INDICES_OFFSET;
			ListSet(perFrustumBuffersHandles, index, *buffer);

			frustums[i].shadedActorWallsCullingInfo = lunaGetBufferDeviceAddress(device,
																				 buffers.actorWalls.shadedCullingInfo);
			frustums[i].shadedActorWallsDrawInfo = lunaGetBufferDeviceAddress(
					device,
					(LunaBuffer)ListGetPointer(buffers.actorWalls.shadedDrawInfo, i));
			frustums[i].shadedActorWallsInstanceIndices = lunaGetBufferDeviceAddress(device, *buffer);
		}
	}
	if (reallocInfo->shouldReallocUnshadedWalls)
	{
		buffers.actorWalls.unshadedInstanceCount = reallocInfo->unshadedWallsInstanceCount;

		const size_t cullingInfoSize = sizeof(uint32_t) +
									   sizeof(CullingInfo) * buffers.actorWalls.unshadedInstanceCount;
		free(unshadedWallsCullingInfo);
		unshadedWallsCullingInfo = malloc(cullingInfoSize);
		CheckAlloc(unshadedWallsCullingInfo);
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorWalls.unshadedCullingInfo,
												cullingInfoSize),
							   "Failed to resize unshaded actor walls culling info buffer!");
		VulkanTestReturnResult(lunaWriteUintToBuffer(device,
													 commandBuffer,
													 buffers.actorWalls.unshadedCullingInfo,
													 0,
													 buffers.actorWalls.unshadedInstanceCount,
													 NULL),
							   "Failed to write unshaded actor walls culling info to buffer!");

		for (uint32_t i = 0; i < frustumCount; i++)
		{
			LunaBuffer *buffer = (LunaBuffer *)&ListGetPointer(buffers.actorWalls.unshadedInstanceIndices, i);
			VulkanTestReturnResult(lunaResizeBuffer(device,
													commandBuffer,
													buffer,
													sizeof(uint32_t) * buffers.actorWalls.unshadedInstanceCount),
								   "Failed to resize unshaded actor walls instance indices buffer!");
			const uint32_t index = PER_FRUSTUM_BUFFER_COUNT * i + PER_FRUSTUM_BUFFER_WALL_ACTOR_INSTANCE_INDICES_OFFSET;
			ListSet(perFrustumBuffersHandles, index + 1, *buffer);

			frustums[i]
					.unshadedActorWallsCullingInfo = lunaGetBufferDeviceAddress(device,
																				buffers.actorWalls.unshadedCullingInfo);
			frustums[i].unshadedActorWallsDrawInfo = lunaGetBufferDeviceAddress(
					device,
					(LunaBuffer)ListGetPointer(buffers.actorWalls.unshadedDrawInfo, i));
			frustums[i].unshadedActorWallsInstanceIndices = lunaGetBufferDeviceAddress(device, *buffer);
		}
	}
	if (reallocInfo->shouldReallocShadedWalls || reallocInfo->shouldReallocUnshadedWalls)
	{
		const size_t instanceDataSize = sizeof(ActorWallInstanceData) * (buffers.actorWalls.shadedInstanceCount +
																		 buffers.actorWalls.unshadedInstanceCount);
		free(wallsInstanceData);
		wallsInstanceData = malloc(instanceDataSize);
		CheckAlloc(wallsInstanceData);
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorWalls.instanceData,
												instanceDataSize),
							   "Failed to resize shaded actor walls instance data buffer!");
		const LunaDescriptorBufferInfo instanceDataBufferInfo = {
			.buffer = buffers.actorWalls.instanceData,
		};
		const LunaWriteDescriptorSet instanceDataWrite = {
			.descriptorSet = descriptorSets.common.set,
			.bindingName = "Actor Instance Data",
			.descriptorArrayElement = 0,
			.descriptorCount = 1,
			.bufferInfos = &instanceDataBufferInfo,
		};
		lunaWriteDescriptorSets(device, 1, &instanceDataWrite);
	}

	allocatedBufferCount = frustumCount;
	VulkanTestReturnResult(WriteFrustumsBuffer(), "Failed to write data to frustums buffer!");

	return VK_SUCCESS;
}

static inline void UpdateActorModelInstanceData(const Actor *actor,
												List *lodInstanceCounts,
												InstanceDataReallocInfo *reallocInfo)
{
	const ModelLod *lod = &actor->model->lods[actor->currentLod];
	const uint32_t lodId = lod->id;
	const LodMaterialSlotsData *materialSlotsData = ListGetPointer(lodMaterialSlotsData, lodId);
	assert(lodInstanceCounts->length == lodMaterialSlotsData.length &&
		   ListGetUint32(*lodInstanceCounts, lodId) <= materialSlotsData->instanceCount);
	assert(actor->model->materialSlotCount == materialSlotsData->materialSlots.length);
	const uint32_t remainingInstances = ListGetUint32(*lodInstanceCounts, lodId) - 1;
	ListSet(*lodInstanceCounts, lodId, remainingInstances);
	mat4 transformMatrix;
	ActorTransformMatrix(actor, &transformMatrix);
	for (uint32_t j = 0; j < materialSlotsData->materialSlots.length; j++)
	{
		const uint32_t materialIndex = actor->model->skinMaterialIndices[actor->currentSkinIndex][j];
		const Material *material = &actor->model->materials[materialIndex];
		const MaterialSlotData *materialSlotData = ListGetPointer(materialSlotsData->materialSlots, j);

		const uint32_t instanceIndex = reallocInfo->shadedModelInstanceCount + reallocInfo->unshadedModelInstanceCount;
		ModelActorCullingInfo *cullingInfo = NULL;
		switch (material->shader)
		{
			case SHADER_SHADED:
				cullingInfo = &shadedModelsCullingInfo[reallocInfo->shadedModelInstanceCount];
				cullingInfo->drawInfoIndex = materialSlotData->shadedDrawInfo - shadedModelsDrawInfo;
				shadedModelsInstanceIndices[reallocInfo->shadedModelInstanceCount] = instanceIndex;
				materialSlotData->unshadedDrawInfo->firstInstance++;
				reallocInfo->shadedModelInstanceCount++;
				break;
			case SHADER_UNSHADED:
				cullingInfo = &unshadedModelsCullingInfo[reallocInfo->unshadedModelInstanceCount];
				cullingInfo->drawInfoIndex = materialSlotData->unshadedDrawInfo - unshadedModelsDrawInfo;
				unshadedModelsInstanceIndices[reallocInfo->unshadedModelInstanceCount] = instanceIndex;
				reallocInfo->unshadedModelInstanceCount++;
				break;
			default:
				assert(material->shader != SHADER_SKY);
				continue;
		}
		ActorModelInstanceData *instanceData = &modelsInstanceData[instanceIndex];
		memcpy(instanceData->transformMatrix, transformMatrix, sizeof(transformMatrix));
		instanceData->modColor = actor->modColor;
		instanceData->materialColor = material->color;
		instanceData->textureIndex = TextureIndex(material->texture);

		Vector3 position;
		JPH_BodyInterface_GetPosition(actor->bodyInterface, actor->bodyId, &position);
		Vector3_Add(&position, &lod->components[j].centerOffset, &cullingInfo->position);
		cullingInfo->radius = lod->components[j].radius;
		cullingInfo->castsShadows = material->castsShadows;
	}
}

static inline void UpdateActorWallInstanceData(const Actor *actor,
											   ActorWallInstanceData *actorInstanceData,
											   CullingInfo *cullingInfo)
{
	JPH_RVec3 position;
	JPH_Quat rotation;
	JPH_BodyInterface_GetPositionAndRotation(actor->bodyInterface, actor->bodyId, &position, &rotation);
	actorInstanceData->position = position;
	actorInstanceData->scale.x = actor->wall->length;
	actorInstanceData->scale.y = actor->wall->height;
	actorInstanceData->axis.x = actor->wall->orientation == ACTOR_WALL_ORIENTATION_X_AXIS ? 1 : 0;
	actorInstanceData->axis.y = actor->wall->orientation == ACTOR_WALL_ORIENTATION_Z_AXIS ? 1 : 0;
	actorInstanceData->centerOffset = actor->wall->centerOffset;
	actorInstanceData->rotationQuat = rotation;
	actorInstanceData->textureIndex = TextureIndex(actor->wall->texture);
	actorInstanceData->uvScale = actor->wall->uvScale;
	actorInstanceData->uvOffset = actor->wall->uvOffset;
	actorInstanceData->modColor = actor->modColor;

	// TODO: This does not take centerOffset into account
	cullingInfo->position = position;
	cullingInfo->radius = Vector2Length(actorInstanceData->scale);
	cullingInfo->castsShadows = true;
}

static inline VkResult WriteActorModelBuffers(const InstanceDataReallocInfo *reallocInfo)
{
	assert(reallocInfo->modelInstanceCount ==
		   reallocInfo->shadedModelInstanceCount + reallocInfo->unshadedModelInstanceCount);

	const LunaBufferWriteInfo instanceDataWriteInfo = {
		.bytes = reallocInfo->modelInstanceCount * sizeof(ActorModelInstanceData),
		.data = modelsInstanceData,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorModels.instanceData,
												 &instanceDataWriteInfo),
						   "Failed to write actor models instance data to buffer!");

	const LunaBufferWriteInfo shadedUnculledInstanceIndicesWriteInfo = {
		.bytes = reallocInfo->shadedModelInstanceCount * sizeof(uint32_t),
		.data = shadedModelsInstanceIndices,
		.stageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorModels.shadedUnculledInstanceIndices,
												 &shadedUnculledInstanceIndicesWriteInfo),
						   "Failed to write shaded actor models unculled instance indices to buffer!");
	const LunaBufferWriteInfo unshadedUnculledInstanceIndicesWriteInfo = {
		.bytes = reallocInfo->unshadedModelInstanceCount * sizeof(uint32_t),
		.data = unshadedModelsInstanceIndices,
		.stageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorModels.unshadedUnculledInstanceIndices,
												 &unshadedUnculledInstanceIndicesWriteInfo),
						   "Failed to write unshaded actor models unculled instance indices to buffer!");

	VulkanTestReturnResult(lunaWriteUintToBuffer(device,
												 commandBuffer,
												 buffers.actorModels.shadedCullingInfo,
												 0,
												 reallocInfo->shadedModelInstanceCount,
												 NULL),
						   "Failed to write actor model shaded culling info count to buffer!");
	const LunaBufferWriteInfo shadedCullingInfoBufferWriteInfo = {
		.bytes = reallocInfo->shadedModelInstanceCount * sizeof(ModelActorCullingInfo),
		.data = shadedModelsCullingInfo,
		.offset = sizeof(uint32_t),
		.stageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorModels.shadedCullingInfo,
												 &shadedCullingInfoBufferWriteInfo),
						   "Failed to write data to actor model shaded culling info buffer!");
	VulkanTestReturnResult(lunaWriteUintToBuffer(device,
												 commandBuffer,
												 buffers.actorModels.unshadedCullingInfo,
												 0,
												 reallocInfo->unshadedModelInstanceCount,
												 NULL),
						   "Failed to write actor model unshaded culling info count to buffer!");
	const LunaBufferWriteInfo unshadedCullingInfoBufferWriteInfo = {
		.bytes = reallocInfo->unshadedModelInstanceCount * sizeof(ModelActorCullingInfo),
		.data = unshadedModelsCullingInfo,
		.offset = sizeof(uint32_t),
		.stageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorModels.unshadedCullingInfo,
												 &unshadedCullingInfoBufferWriteInfo),
						   "Failed to write data to actor model unshaded culling info buffer!");

	const uint32_t drawInfoBytes = reallocInfo->modelDrawCount * sizeof(VkDrawIndexedIndirectCommand);
	const LunaBufferWriteInfo shadedDrawInfoBufferWriteInfo = {
		.bytes = drawInfoBytes,
		.data = shadedModelsDrawInfo,
		.stageFlags = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
	};
	const LunaBufferWriteInfo unshadedDrawInfoBufferWriteInfo = {
		.bytes = drawInfoBytes,
		.data = unshadedModelsDrawInfo,
		.stageFlags = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
	};
	for (uint32_t i = 0; i < frustumCount; i++)
	{
		const LunaBuffer shadedDrawInfo = (LunaBuffer)ListGetPointer(buffers.actorModels.shadedDrawInfo, i);
		const LunaBuffer unshadedDrawInfo = (LunaBuffer)ListGetPointer(buffers.actorModels.unshadedDrawInfo, i);
		VulkanTestReturnResult(lunaWriteDataToBuffer(device,
													 commandBuffer,
													 shadedDrawInfo,
													 &shadedDrawInfoBufferWriteInfo),
							   "Failed to write actor models shaded draw info to buffer!");
		VulkanTestReturnResult(lunaWriteDataToBuffer(device,
													 commandBuffer,
													 unshadedDrawInfo,
													 &unshadedDrawInfoBufferWriteInfo),
							   "Failed to write actor models unshaded draw info to buffer!");
	}

	return VK_SUCCESS;
}

static inline VkResult WriteActorWallBuffers()
{
	const LunaBufferWriteInfo instanceDataWriteInfo = {
		.bytes = sizeof(ActorWallInstanceData) *
				 (buffers.actorWalls.shadedInstanceCount + buffers.actorWalls.unshadedInstanceCount),
		.data = wallsInstanceData,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorWalls.instanceData,
												 &instanceDataWriteInfo),
						   "Failed to write actor walls instance data to buffer!");

	const LunaBufferWriteInfo shadedWallsCullingInfoWriteInfo = {
		.bytes = sizeof(CullingInfo) * buffers.actorWalls.shadedInstanceCount,
		.offset = sizeof(uint32_t),
		.data = shadedWallsCullingInfo,
		.stageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorWalls.shadedCullingInfo,
												 &shadedWallsCullingInfoWriteInfo),
						   "Failed to write shaded actor walls culling info to buffer!");

	const LunaBufferWriteInfo unshadedWallsCullingInfoWriteInfo = {
		.bytes = sizeof(CullingInfo) * buffers.actorWalls.unshadedInstanceCount,
		.offset = sizeof(uint32_t),
		.data = unshadedWallsCullingInfo,
		.stageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorWalls.unshadedCullingInfo,
												 &unshadedWallsCullingInfoWriteInfo),
						   "Failed to write unshaded actor walls culling info to buffer!");

	return VK_SUCCESS;
}

static inline VkResult UpdateInstanceData(const LockingList *actors,
										  List *lodInstanceCounts,
										  InstanceDataReallocInfo *reallocInfo)
{
	size_t shadedWallsInstanceIndex = 0;
	size_t unshadedWallsInstanceIndex = 0;
	for (size_t i = 0; i < actors->length; i++)
	{
		const Actor *actor = ListGetPointer(*actors, i);
		if (!actor->visible)
		{
			continue;
		}
		if (actor->hasModel)
		{
			UpdateActorModelInstanceData(actor, lodInstanceCounts, reallocInfo);
		} else if (actor->wall)
		{
			if (actor->wall->unshaded)
			{
				UpdateActorWallInstanceData(actor,
											&wallsInstanceData[unshadedWallsInstanceIndex +
															   buffers.actorWalls.shadedInstanceCount],
											&unshadedWallsCullingInfo[unshadedWallsInstanceIndex]);
				unshadedWallsInstanceIndex++;
			} else
			{
				UpdateActorWallInstanceData(actor,
											&wallsInstanceData[shadedWallsInstanceIndex],
											&shadedWallsCullingInfo[shadedWallsInstanceIndex]);
				shadedWallsInstanceIndex++;
			}
		}
	}
	maximumCulledInstanceCount = max(maximumCulledInstanceCount, reallocInfo->shadedWallsInstanceCount);
	maximumCulledInstanceCount = max(maximumCulledInstanceCount, reallocInfo->unshadedWallsInstanceCount);
	maximumCulledInstanceCount = max(maximumCulledInstanceCount, reallocInfo->shadedModelInstanceCount);
	maximumCulledInstanceCount = max(maximumCulledInstanceCount, reallocInfo->unshadedModelInstanceCount);

	VulkanTestReturnResult(WriteActorModelBuffers(reallocInfo), "Failed to write actor model buffers!");

	VulkanTestReturnResult(WriteActorWallBuffers(), "Failed to write actor wall buffers!");

	const LunaMultiBufferMemoryBarrier memoryBarrier = {
		.sourceStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		.sourceAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		.destinationAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
		.bufferCount = perFrustumBuffersHandles.length,
		.buffers = (LunaBuffer *)perFrustumBuffersHandles.data->pointerData,
	};
	const LunaDependencyInfo dependencyInfo = {
		.multiBufferMemoryBarrierCount = 1,
		.multiBufferMemoryBarriers = &memoryBarrier,
	};
	VulkanTestReturnResult(lunaPipelineBarrier(device, commandBuffer, &dependencyInfo),
						   "Failed to insert pipeline barrier after writing actor model instance data!");

	return VK_SUCCESS;
}

VkResult UpdateActors()
{
	const LockingList *actors = &GetState()->map->actors;
	ListLock(*actors);
	List lodInstanceCounts;
	InstanceDataReallocInfo reallocInfo = {
		.lodInstanceCounts = &lodInstanceCounts,
	};
	if (ShouldReallocInstanceData(actors, &reallocInfo))
	{
		VulkanTestReturnResult(ReallocateInstanceData(actors, &reallocInfo),
							   "Failed to reallocate actor instance data!");
	}
	VulkanTestReturnResult(UpdateInstanceData(actors, &lodInstanceCounts, &reallocInfo),
						   "Failed to update actor models instance data!");
	ListUnlock(*actors);

	ListFree(lodInstanceCounts);

	return VK_SUCCESS;
}

bool ClearModelCache()
{
	ListClear(loadedModelIds);
	ClearLodMaterialSlotsData();
	for (size_t i = 0; i < lodMaterialSlotsVertexData.length; i++)
	{
		ListAndContentsFree(ListGetNestedList(lodMaterialSlotsVertexData, i));
	}
	ListClear(lodMaterialSlotsVertexData);
	VulkanTest(lunaResizeBuffer(device, commandBuffer, &buffers.actorModels.vertices, 0),
			   "Failed to empty actor model vertex buffer!");
	VulkanTest(lunaResizeBuffer(device, commandBuffer, &buffers.actorModels.indices, 0),
			   "Failed to empty actor model index buffer!");
	bufferVertexCount = 0;
	bufferIndexCount = 0;

	return true;
}
