//
// Created by NBT22 on 10/20/26.
//

#include <assert.h>
#include <engine/assets/ModelLoader.h>
#include <engine/graphics/vulkan/VulkanActors.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanModel.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaTypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

List loadedModelIds = {0};
List lodMaterialSlotsVertexData = {0};
uint32_t allocatedBufferCount = 0;
size_t bufferVertexCount = 0;
size_t bufferIndexCount = 0;

/// A CPU copy of the contents of the models vertex buffer
static ModelVertex *vertices = NULL;
/// A CPU copy of the contents of the models index buffer
static uint32_t *indices = NULL;

void LoadModelLods(const ModelDefinition *model)
{
	if (ListFind(loadedModelIds, model->id) != SIZE_MAX)
	{
		// Model is already loaded, so we're done here
		return;
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
		// No vertices to load? Don't load it
		return;
	}

	const size_t verticesSize = lunaGetBufferSize(buffers.modelVertices) + vertexCount * sizeof(ModelVertex);
	const size_t indicesSize = lunaGetBufferSize(buffers.modelIndices) + indexCount * sizeof(uint32_t);

	void *newVertices = realloc(vertices, verticesSize);
	CheckAlloc(newVertices);
	vertices = newVertices;
	ModelVertex *vertexData = newVertices + lunaGetBufferSize(buffers.modelVertices);

	void *newIndices = realloc(indices, indicesSize);
	CheckAlloc(newIndices);
	indices = newIndices;
	uint32_t *indexData = newIndices + lunaGetBufferSize(buffers.modelIndices);

	VulkanTest(lunaResizeBuffer(device, &buffers.modelVertices, verticesSize),
			   "Failed to resize actor model vertex buffer!");
	VulkanTest(lunaResizeBuffer(device, &buffers.modelIndices, indicesSize),
			   "Failed to resize actor model index buffer!");

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
		.size = verticesSize,
		.data = vertices,
		.synchronizationType = LUNA_BUFFER_SYNCHRONIZATION_TYPE_VERTEX_BUFFER,
	};
	VulkanTest(lunaWriteDataToBuffer(device, commandBuffer, buffers.modelVertices, &vertexBufferWriteInfo),
			   "Failed to write model vertex data to buffer!");
	const LunaBufferWriteInfo indexBufferWriteInfo = {
		.size = indicesSize,
		.data = indices,
		.synchronizationType = LUNA_BUFFER_SYNCHRONIZATION_TYPE_INDEX_BUFFER,
	};
	VulkanTest(lunaWriteDataToBuffer(device, commandBuffer, buffers.modelIndices, &indexBufferWriteInfo),
			   "Failed to write model index data to buffer!");

	bufferVertexCount += vertexCount;
	bufferIndexCount += indexCount;
}

void ClearModelCache()
{
	ListClear(loadedModelIds);
	ClearActorLodMaterialSlotsData();
	for (size_t i = 0; i < lodMaterialSlotsVertexData.length; i++)
	{
		ListAndContentsFree(ListGetNestedList(lodMaterialSlotsVertexData, i));
	}
	ListClear(lodMaterialSlotsVertexData);
	VulkanTest(lunaResizeBuffer(device, &buffers.modelVertices, 0), "Failed to empty model vertex buffer!");
	VulkanTest(lunaResizeBuffer(device, &buffers.modelIndices, 0), "Failed to empty model index buffer!");
	bufferVertexCount = 0;
	bufferIndexCount = 0;
	free(vertices);
	vertices = NULL;
	free(indices);
	indices = NULL;
}
