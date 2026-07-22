// Created by NBT22 on 1/23/26.
//

#include <assert.h>
#include <engine/assets/ModelLoader.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/graphics/vulkan/VulkanActors.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/RVec3.h>
#include <joltc/Physics/Body/BodyInterface.h>
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
	/// A pointer to the instance data for the first instance
	ActorModelInstanceData *instanceData;
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
	uint32_t modelDrawCount;
	uint32_t shadedWallsInstanceCount;
	uint32_t unshadedWallsInstanceCount;
	bool shouldReallocModels;
	bool shouldReallocShadedWalls;
	bool shouldReallocUnshadedWalls;
} InstanceDataReallocInfo;

static size_t bufferVertexCount;
static size_t bufferIndexCount;
static ActorModelInstanceData *modelsInstanceData;
static VkDrawIndexedIndirectCommand *shadedModelsDrawInfo;
static VkDrawIndexedIndirectCommand *unshadedModelsDrawInfo;
static ActorWallInstanceData *shadedWallsInstanceData;
static ActorWallInstanceData *unshadedWallsInstanceData;

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
		const LodMaterialSlotsData *materialSlotData = ListGetPointer(lodMaterialSlotsData, i);
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
			materialSlotVertexData->indexCount = model->lods[i].indexCount[j];
			materialSlotVertexData->firstIndex = bufferIndexCount + indexCount;
			materialSlotVertexData->vertexOffset = (int32_t)(bufferVertexCount + vertexCount);
			ListAdd(*materialSlotsVertexData, materialSlotVertexData);

			indexCount += model->lods[i].indexCount[j];
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
			memcpy(indexData + indexOffset,
				   lod->indexData[materialSlotIndex],
				   lod->indexCount[materialSlotIndex] * sizeof(uint32_t));
			indexOffset += lod->indexCount[materialSlotIndex];
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

static inline VkResult LoadActor(const Actor *actor)
{
	if (actor->hasModel)
	{
		return LoadModelLods(actor->model);
	}
	return VK_SUCCESS;
}

VkResult LoadActors(const LockingList *actors)
{
	buffers.actorWalls.shadedInstanceCount = 0;
	buffers.actorWalls.unshadedInstanceCount = 0;

	ListLock(*actors);
	for (size_t i = 0; i < actors->length; i++)
	{
		VulkanTestReturnResult(LoadActor(ListGetPointer(*actors, i)), "Failed to load actor!");
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
				reallocInfo->modelDrawCount += actor->model->materialSlotCount;
			}
			ListSet(*reallocInfo->lodInstanceCounts, lodId, ListGetUint32(*reallocInfo->lodInstanceCounts, lodId) + 1);
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
	const uint32_t drawInfoBytes = reallocInfo->modelDrawCount * sizeof(VkDrawIndexedIndirectCommand);

	if (reallocInfo->shouldReallocModels)
	{
		free(shadedModelsDrawInfo);
		free(unshadedModelsDrawInfo);
		free(modelsInstanceData); // Intentionally not using realloc in order to skip the memmove associated with it
		shadedModelsDrawInfo = calloc(1, drawInfoBytes);
		CheckAlloc(shadedModelsDrawInfo);
		unshadedModelsDrawInfo = calloc(1, drawInfoBytes);
		CheckAlloc(unshadedModelsDrawInfo);
		modelsInstanceData = malloc(reallocInfo->modelInstanceCount * sizeof(ActorModelInstanceData));
		CheckAlloc(modelsInstanceData);

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
					materialSlotData->shadedDrawInfo->instanceCount = materialSlotsData->instanceCount;
					materialSlotData->shadedDrawInfo->firstIndex = materialSlotVertexData->firstIndex;
					materialSlotData->shadedDrawInfo->vertexOffset = materialSlotVertexData->vertexOffset;
					materialSlotData->shadedDrawInfo->firstInstance = instanceDataOffset;
					materialSlotData->unshadedDrawInfo = unshadedModelsDrawInfo + drawInfoOffset;
					*materialSlotData->unshadedDrawInfo = *materialSlotData->shadedDrawInfo;
					materialSlotData->instanceData = modelsInstanceData + instanceDataOffset;
					ListAdd(materialSlotsData->materialSlots, materialSlotData);
					drawInfoOffset++;
					instanceDataOffset += materialSlotsData->instanceCount;
				}
			}
		}
	}

	if (reallocInfo->shouldReallocModels)
	{
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorModels.instanceData,
												instanceDataOffset * sizeof(ActorModelInstanceData)),
							   "Failed to resize actor models instance data buffer!");
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorModels.shadedDrawInfo,
												drawInfoBytes),
							   "Failed to resize actor models shaded draw info buffer!");
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorModels.unshadedDrawInfo,
												drawInfoBytes),
							   "Failed to resize actor models unshaded draw info buffer!");

		const LunaBufferWriteInfo shadedWriteInfo = {
			.bytes = drawInfoBytes,
			.data = shadedModelsDrawInfo,
			.stageFlags = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		};
		VulkanTestReturnResult(lunaWriteDataToBuffer(device,
													 commandBuffer,
													 buffers.actorModels.shadedDrawInfo,
													 &shadedWriteInfo),
							   "Failed to write actor models shaded draw info to buffer!");
		const LunaBufferWriteInfo unshadedWriteInfo = {
			.bytes = drawInfoBytes,
			.data = unshadedModelsDrawInfo,
			.stageFlags = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		};
		VulkanTestReturnResult(lunaWriteDataToBuffer(device,
													 commandBuffer,
													 buffers.actorModels.unshadedDrawInfo,
													 &unshadedWriteInfo),
							   "Failed to write actor models unshaded draw info to buffer!");
	}

	if (reallocInfo->shouldReallocShadedWalls)
	{
		buffers.actorWalls.shadedInstanceCount = reallocInfo->shadedWallsInstanceCount;
		free(shadedWallsInstanceData);
		shadedWallsInstanceData = malloc(sizeof(ActorWallInstanceData) * buffers.actorWalls.shadedInstanceCount);
		CheckAlloc(shadedWallsInstanceData);
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorWalls.shadedInstanceData,
												sizeof(ActorWallInstanceData) * buffers.actorWalls.shadedInstanceCount),
							   "Failed to resize shaded actor walls instance data buffer!");
	}
	if (reallocInfo->shouldReallocUnshadedWalls)
	{
		buffers.actorWalls.unshadedInstanceCount = reallocInfo->unshadedWallsInstanceCount;
		free(unshadedWallsInstanceData);
		unshadedWallsInstanceData = malloc(sizeof(ActorWallInstanceData) * buffers.actorWalls.unshadedInstanceCount);
		CheckAlloc(unshadedWallsInstanceData);
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												&buffers.actorWalls.unshadedInstanceData,
												sizeof(ActorWallInstanceData) *
														buffers.actorWalls.unshadedInstanceCount),
							   "Failed to resize unshaded actor walls instance data buffer!");
	}

	return VK_SUCCESS;
}

static inline void UpdateActorModelInstanceData(const Actor *actor, List *lodInstanceCounts)
{
	const uint32_t lodId = actor->model->lods[actor->currentLod].id;
	const LodMaterialSlotsData *materialSlotsData = ListGetPointer(lodMaterialSlotsData, lodId);
	assert(lodInstanceCounts->length == lodMaterialSlotsData.length &&
		   ListGetUint32(*lodInstanceCounts, lodId) <= materialSlotsData->instanceCount);
	assert(actor->model->materialSlotCount == materialSlotsData->materialSlots.length);
	const uint32_t instanceIndex = ListGetUint32(*lodInstanceCounts, lodId) - 1;
	ListSet(*lodInstanceCounts, lodId, instanceIndex);
	mat4 transformMatrix;
	ActorTransformMatrix(actor, &transformMatrix);
	for (uint32_t j = 0; j < materialSlotsData->materialSlots.length; j++)
	{
		const uint32_t materialIndex = actor->model->skinMaterialIndices[actor->currentSkinIndex][j];
		const Material *material = &actor->model->materials[materialIndex];
		const MaterialSlotData *materialSlotData = ListGetPointer(materialSlotsData->materialSlots, j);
		ActorModelInstanceData *instanceData = &materialSlotData->instanceData[instanceIndex];
		memcpy(instanceData->transformMatrix, transformMatrix, sizeof(transformMatrix));
		memcpy(instanceData->modColor, &actor->modColor, sizeof(Color));
		memcpy(instanceData->materialColor, &material->color, sizeof(Color));
		instanceData->textureIndex = TextureIndex(material->texture);
	}
}

static inline void UpdateActorWallInstanceData(const Actor *actor, ActorWallInstanceData *actorInstanceData)
{
	JPH_RVec3 position;
	JPH_Quat rotation;
	JPH_BodyInterface_GetPositionAndRotation(actor->bodyInterface, actor->bodyId, &position, &rotation);
	const Vector2 axis = {
		.x = actor->wall->orientation == ACTOR_WALL_ORIENTATION_X_AXIS ? 1 : 0,
		.y = actor->wall->orientation == ACTOR_WALL_ORIENTATION_Z_AXIS ? 1 : 0,
	};
	const ActorWallInstanceData instanceData = {
		.position.x = position.x,
		.position.y = position.y,
		.position.z = position.z,
		.scale.x = actor->wall->length,
		.scale.y = actor->wall->height,
		.axis = axis,
		.centerOffset = actor->wall->centerOffset,
		.rotationQuat = rotation,
		.textureIndex = TextureIndex(actor->wall->texture),
		.uvScale = actor->wall->uvScale,
		.uvOffset = actor->wall->uvOffset,
		.modColor = actor->modColor,
	};
	memcpy(actorInstanceData, &instanceData, sizeof(instanceData));
}

static inline VkResult WriteWallsInstanceData()
{
	const LunaBufferWriteInfo shadedWallsInstanceDataWriteInfo = {
		.bytes = sizeof(ActorWallInstanceData) * buffers.actorWalls.shadedInstanceCount,
		.data = shadedWallsInstanceData,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorWalls.shadedInstanceData,
												 &shadedWallsInstanceDataWriteInfo),
						   "Failed to write shaded actor walls instance data to buffer!");

	const LunaBufferWriteInfo unshadedWallsInstanceDataWriteInfo = {
		.bytes = sizeof(ActorWallInstanceData) * buffers.actorWalls.unshadedInstanceCount,
		.data = unshadedWallsInstanceData,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorWalls.unshadedInstanceData,
												 &unshadedWallsInstanceDataWriteInfo),
						   "Failed to write unshaded actor walls instance data to buffer!");

	return VK_SUCCESS;
}

static inline VkResult UpdateInstanceData(const LockingList *actors,
										  List *lodInstanceCounts,
										  const uint32_t modelInstanceCount)
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
			UpdateActorModelInstanceData(actor, lodInstanceCounts);
		} else if (actor->wall)
		{
			if (actor->wall->unshaded)
			{
				UpdateActorWallInstanceData(actor, &unshadedWallsInstanceData[unshadedWallsInstanceIndex]);
				unshadedWallsInstanceIndex++;
			} else
			{
				UpdateActorWallInstanceData(actor, &shadedWallsInstanceData[shadedWallsInstanceIndex]);
				shadedWallsInstanceIndex++;
			}
		}
	}
	const LunaBufferWriteInfo modelsInstanceDataWriteInfo = {
		.bytes = modelInstanceCount * sizeof(ActorModelInstanceData),
		.data = modelsInstanceData,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorModels.instanceData,
												 &modelsInstanceDataWriteInfo),
						   "Failed to write actor models instance data to buffer!");

	VulkanTestReturnResult(WriteWallsInstanceData(), "Failed to write actor walls instance data!");

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
	VulkanTestReturnResult(UpdateInstanceData(actors, &lodInstanceCounts, reallocInfo.modelInstanceCount),
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
