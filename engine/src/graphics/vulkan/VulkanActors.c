// Created by NBT22 on 1/23/26.
//

#include <assert.h>
#include <engine/assets/ModelLoader.h>
#include <engine/graphics/vulkan/VulkanActors.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/structs/Actor.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaTypes.h>
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

static size_t bufferVertexCount;
static size_t bufferIndexCount;
static ActorModelInstanceData *modelsInstanceData;
static VkDrawIndexedIndirectCommand *shadedModelsDrawInfo;
static VkDrawIndexedIndirectCommand *unshadedModelsDrawInfo;

/// A list of uint32_t model ids that are currently loaded
static List loadedModelIds;
/// A list, indexed with a lod id, that contains lists of @c MaterialSlotVertexData structures for each material slot
static List lodMaterialSlotsVertexData;
/// A list of @c LodMaterialSlotsData structures, indexed using a lod id
static List lodMaterialSlotsData;

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

	// TODO: Load non-model actors

	return VK_SUCCESS;
}

VkResult LoadActors(const LockingList *actors)
{
	ListLock(*actors);
	for (size_t i = 0; i < actors->length; i++)
	{
		VulkanTestReturnResult(LoadActor(ListGetPointer(*actors, i)), "Failed to load actor!");
	}
	ListUnlock(*actors);

	return VK_SUCCESS;
}

static inline bool ShouldReallocInstanceData(const LockingList *actors,
											 List *lodInstanceCounts,
											 uint32_t *totalInstanceCount,
											 uint32_t *drawCount)
{
	ListInit(*lodInstanceCounts, LIST_UINT32);
	for (size_t i = 0; i < actors->length; i++)
	{
		const Actor *actor = ListGetPointer(*actors, i);
		if (!actor->hasModel)
		{
			continue;
		}
		*totalInstanceCount += actor->model->materialSlotCount;
		const uint32_t lodId = actor->model->lods[actor->currentLod].id;
		while (lodId >= lodInstanceCounts->length)
		{
			ListAdd(*lodInstanceCounts, 0);
			*drawCount += actor->model->materialSlotCount;
		}
		ListSet(*lodInstanceCounts, lodId, ListGetUint32(*lodInstanceCounts, lodId) + 1);
	}

	if (lodInstanceCounts->length != lodMaterialSlotsData.length)
	{
		return true;
	}

	for (uint32_t i = 0; i < lodInstanceCounts->length; i++)
	{
		const uint32_t instanceCount = ListGetUint32(*lodInstanceCounts, i);
		const LodMaterialSlotsData *materialSlotDatas = ListGetPointer(lodMaterialSlotsData, i);
		if (instanceCount != materialSlotDatas->instanceCount)
		{
			return true;
		}
	}

	return false;
}

static inline VkResult ReallocateInstanceData(const LockingList *actors,
											  List *lodInstanceCounts,
											  const uint32_t totalInstanceCount,
											  const uint32_t drawCount)
{
	const uint32_t drawInfoBytes = drawCount * sizeof(VkDrawIndexedIndirectCommand);

	free(shadedModelsDrawInfo);
	free(unshadedModelsDrawInfo);
	free(modelsInstanceData); // Intentionally not using realloc in order to skip the memmove associated with it
	shadedModelsDrawInfo = calloc(1, drawInfoBytes);
	CheckAlloc(shadedModelsDrawInfo);
	unshadedModelsDrawInfo = calloc(1, drawInfoBytes);
	CheckAlloc(unshadedModelsDrawInfo);
	modelsInstanceData = malloc(totalInstanceCount * sizeof(ActorModelInstanceData));
	CheckAlloc(modelsInstanceData);

	ListFree(lodMaterialSlotsData);
	ListInit(lodMaterialSlotsData, LIST_POINTER);
	for (size_t i = 0; i < lodInstanceCounts->length; i++)
	{
		LodMaterialSlotsData *materialSlotsData = malloc(sizeof(LodMaterialSlotsData));
		materialSlotsData->instanceCount = ListGetUint32(*lodInstanceCounts, i);
		ListInit(materialSlotsData->materialSlots, LIST_POINTER);
		ListAdd(lodMaterialSlotsData, materialSlotsData);
	}

	size_t instanceDataOffset = 0;
	size_t drawInfoOffset = 0;
	for (size_t i = 0; i < actors->length; i++)
	{
		const Actor *actor = ListGetPointer(*actors, i);
		if (!actor->hasModel)
		{
			continue;
		}

		const uint32_t lodId = actor->model->lods[actor->currentLod].id;
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

	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffers.actorModels.instanceData,
											instanceDataOffset * sizeof(ActorModelInstanceData)),
						   "Failed to grow actor models instance data buffer!");
	VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.actorModels.shadedDrawInfo, drawInfoBytes),
						   "Failed to grow actor models shaded draw info buffer!");
	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffers.actorModels.unshadedDrawInfo,
											drawInfoBytes),
						   "Failed to grow actor models unshaded draw info buffer!");

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

	return VK_SUCCESS;
}

static inline VkResult UpdateInstanceData(const LockingList *actors,
										  List *lodInstanceCounts,
										  const uint32_t totalInstanceCount)
{
	for (size_t i = 0; i < actors->length; i++)
	{
		const Actor *actor = ListGetPointer(*actors, i);
		if (!actor->hasModel)
		{
			continue;
		}

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
	const LunaBufferWriteInfo writeInfo = {
		.bytes = totalInstanceCount * sizeof(ActorModelInstanceData),
		.data = modelsInstanceData,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.actorModels.instanceData, &writeInfo),
						   "Failed to write actor models instance data to buffer!");
	return VK_SUCCESS;
}

VkResult UpdateActors()
{
	const LockingList *actors = &GetState()->map->actors;
	ListLock(*actors);
	List lodInstanceCounts;
	uint32_t totalInstanceCount = 0;
	uint32_t drawCount = 0;
	if (ShouldReallocInstanceData(actors, &lodInstanceCounts, &totalInstanceCount, &drawCount))
	{
		VulkanTestReturnResult(ReallocateInstanceData(actors, &lodInstanceCounts, totalInstanceCount, drawCount),
							   "Failed to reallocate actor models instance data!");
	}
	VulkanTestReturnResult(UpdateInstanceData(actors, &lodInstanceCounts, totalInstanceCount),
						   "Failed to update actor models instance data!");
	ListUnlock(*actors);

	return VK_SUCCESS;
}
