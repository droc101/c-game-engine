//
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

#define MODEL_LOD_ID(modelId, lodId) ((((uint64_t)(modelId)) << 32) | ((uint64_t)(lodId)))
#define MODEL_LOD_DATA(_modelId, _lodId) ((ModelLodData[]){{.modelId = (_modelId), .lodId = (_lodId)}})
#define INSTANCE_DATA_OFFSET(_actorId) ((InstanceDataOffset[]){{.actorId = (_actorId)}})

typedef struct
{
	uint32_t modelId;
	uint32_t lodId;
	size_t vertexOffset;
	size_t indexOffset;
	size_t drawInfoOffset;
} ModelLodData;

typedef struct
{
	uint64_t actorId;
	uint64_t materialId;
	size_t instanceDataOffset;
} InstanceDataOffset;

static size_t bufferVertexOffset;
static size_t bufferIndexOffset;
static ActorModelInstanceData *modelsInstanceData;
static VkDrawIndexedIndirectCommand *shadedModelsDrawInfo;
static VkDrawIndexedIndirectCommand *unshadedModelsDrawInfo;

static SortedList modelLods;
static SortedList instanceDataOffsets;

static inline int CompareModelLodDatas(const void *a, const void *b)
{
	const ModelLodData *dataA = a;
	const ModelLodData *dataB = b;
	const uint64_t idA = MODEL_LOD_ID(dataA->modelId, dataB->modelId);
	const uint64_t idB = MODEL_LOD_ID(dataB->modelId, dataB->modelId);
	return (idA > idB) - (idA < idB);
}

static inline int CompareInstanceDataOffsets(const void *a, const void *b)
{
	const InstanceDataOffset *dataA = a;
	const InstanceDataOffset *dataB = b;
	return (dataA->actorId > dataB->actorId) - (dataA->actorId < dataB->actorId);
}

void InitActorLoadingVariables()
{
	ListInit(modelLods, ModelLodData, CompareModelLodDatas);
	ListInit(instanceDataOffsets, InstanceDataOffset, CompareInstanceDataOffsets);
}

static inline VkResult LoadModelLods(const ModelDefinition *model)
{
	size_t vertexSize = 0;
	size_t indexSize = 0;
	for (uint32_t i = 0; i < model->lodCount; i++)
	{
		const ModelLod *lod = model->lods + i;
		if (ListFind(modelLods, MODEL_LOD_DATA(model->id, lod->id)) == SIZE_MAX)
		{
			vertexSize += lod->vertexCount * sizeof(ModelVertex);
			indexSize += lod->totalIndexCount * sizeof(uint32_t);
		}
	}
	if (vertexSize == 0 || indexSize == 0)
	{
		assert(vertexSize == indexSize);
		return VK_SUCCESS;
	}

	VulkanTestReturnResult(lunaGrowBuffer(device,
										  commandBuffer,
										  &buffers.actorModels.vertices,
										  lunaGetBufferSize(buffers.actorModels.vertices) + vertexSize),
						   "Failed to resize actor model vertex buffer!");
	VulkanTestReturnResult(lunaGrowBuffer(device,
										  commandBuffer,
										  &buffers.actorModels.indices,
										  lunaGetBufferSize(buffers.actorModels.indices) + indexSize),
						   "Failed to resize actor model index buffer!");

	ModelVertex *vertexData = malloc(vertexSize);
	uint32_t *indexData = malloc(indexSize);

	size_t vertexOffset = 0;
	size_t indexOffset = 0;
	for (uint32_t i = 0; i < model->lodCount; i++)
	{
		const ModelLod *lod = model->lods + i;
		const uint64_t modelLodId = MODEL_LOD_ID(model->id, lod->id);
		if (ListFind(modelLods, modelLodId) != SIZE_MAX)
		{
			continue;
		}
		ModelLodData *data = malloc(sizeof(ModelLodData));
		data->modelId = model->id;
		data->lodId = lod->id;
		data->vertexOffset = bufferVertexOffset;
		data->indexOffset = bufferIndexOffset;
		ListAdd(modelLods, data);

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
		.bytes = vertexSize,
		.data = vertexData,
		.offset = bufferVertexOffset,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorModels.vertices,
												 &vertexBufferWriteInfo),
						   "Failed to write model vertex data to buffer!");
	const LunaBufferWriteInfo indexBufferWriteInfo = {
		.bytes = indexSize,
		.data = indexData,
		.offset = bufferIndexOffset,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorModels.indices,
												 &indexBufferWriteInfo),
						   "Failed to write model index data to buffer!");

	bufferVertexOffset += vertexSize;
	bufferIndexOffset += indexSize;

	free(vertexData);
	free(indexData);

	return VK_SUCCESS;
}

static inline VkResult LoadActor(const Actor *actor)
{
	const size_t instanceDataOffsetIndex = ListFind(instanceDataOffsets, INSTANCE_DATA_OFFSET(actor->id));
	if (instanceDataOffsetIndex != -1u)
	{
		// No need to reload lod data, since it must already be loaded

		// const size_t index = ((InstanceDataOffset *)ListGetPointer(instanceDataOffsets, instanceDataOffsetIndex))
		// 							 ->instanceDataOffset;
		// modelsInstanceData[index].;

		return VK_SUCCESS;
	}
	// TODO: Load instance data and draw data
	if (actor->model)
	{
		return LoadModelLods(actor->model);
	}

	// TODO: Load non-model actors

	return VK_SUCCESS;
}

VkResult LoadActors()
{
	const LockingList actors = GetState()->map->actors;
	ListLock(actors);
	for (size_t i = 0; i < actors.length; i++)
	{
		VulkanTestReturnResult(LoadActor(ListGet(actors, i, const Actor *)), "Failed to load actor!");
	}
	ListUnlock(actors);

	return VK_SUCCESS;
}
