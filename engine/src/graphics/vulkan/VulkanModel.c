//
// Created by NBT22 on 10/20/26.
//

#include <assert.h>
#include <engine/assets/ModelLoader.h>
#include <engine/graphics/vulkan/VulkanActors.h>
#include <engine/graphics/vulkan/VulkanDebug.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanModel.h>
#include <engine/helpers/Macros.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaTypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>

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

	VK_BeginDebugLabel("Loading model '%s'", model->name);

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

	VK_EndDebugLabel();
}

void LoadModelToBuffer(const ModelDefinition *model,
					   const uint32_t lodIndex,
					   const uint32_t skinIndex,
					   ModelBuffer *buffer)
{
	assert(buffer != NULL);

	if (model == NULL)
	{
		VulkanTest(lunaResizeBuffer(device, &buffer->instanceData, 0), "Failed to resize instance data buffer!");
		VulkanTest(lunaResizeBuffer(device, &buffer->shadedDrawInfo, 0),
				   "Failed to resize shaded material draw info buffer!");
		VulkanTest(lunaResizeBuffer(device, &buffer->unshadedDrawInfo, 0),
				   "Failed to resize unshaded material draw info buffer!");
		return;
	}

	VK_BeginDebugLabel("Loading model '%s' into buffer", model->name);

	LoadModelLods(model);

	const uint32_t *materialIndices = model->skinMaterialIndices[skinIndex];

	size_t shadedMaterialCount = 0;
	size_t unshadedMaterialCount = 0;
	for (size_t i = 0; i < model->materialSlotCount; i++)
	{
		if (model->materials[materialIndices[i]].shader == SHADER_SHADED)
		{
			shadedMaterialCount++;
		} else
		{
			assert(model->materials[materialIndices[i]].shader == SHADER_UNSHADED);
			unshadedMaterialCount++;
		}
	}

	VulkanTest(lunaResizeBuffer(device,
								&buffer->instanceData,
								(shadedMaterialCount + unshadedMaterialCount) * sizeof(ModelInstanceData)),
			   "Failed to resize instance data buffer!");
	VulkanTest(lunaResizeBuffer(device,
								&buffer->shadedDrawInfo,
								shadedMaterialCount * sizeof(VkDrawIndexedIndirectCommand)),
			   "Failed to resize shaded material draw info buffer!");
	VulkanTest(lunaResizeBuffer(device,
								&buffer->unshadedDrawInfo,
								unshadedMaterialCount * sizeof(VkDrawIndexedIndirectCommand)),
			   "Failed to resize unshaded material draw info buffer!");

	uint32_t shadedCount = 0;
	List *materialSlotsVertexData = &ListGetNestedList(lodMaterialSlotsVertexData, model->lods[lodIndex].id);
	for (uint32_t i = 0; i < model->materialSlotCount; i++)
	{
		const MaterialSlotVertexData *materialSlotVertexData = ListGetPointer(*materialSlotsVertexData, i);
		const Material *material = model->materials + materialIndices[i];

		ModelInstanceData instanceData;
		instanceData.materialColor = material->color;
		instanceData.textureIndex = TextureIndex(material->texture);
		const LunaBufferWriteInfo instanceDataBufferWriteInfo = {
			.size = SizeofMember(ModelInstanceData, materialColor) + SizeofMember(ModelInstanceData, textureIndex),
			.data = &(instanceData.materialColor),
			.offset = i * sizeof(ModelInstanceData) + offsetof(ModelInstanceData, materialColor),
			.synchronizationType = LUNA_BUFFER_SYNCHRONIZATION_TYPE_VERTEX_BUFFER,
		};
		VulkanTest(lunaWriteDataToBuffer(device, commandBuffer, buffer->instanceData, &instanceDataBufferWriteInfo),
				   "Failed to write instance data to buffer!");

		VkDrawIndexedIndirectCommand drawInfo = {
			.indexCount = materialSlotVertexData->indexCount,
			.instanceCount = 1,
			.firstIndex = materialSlotVertexData->firstIndex,
			.vertexOffset = materialSlotVertexData->vertexOffset,
			.firstInstance = i,
		};
		const LunaBufferWriteInfo drawInfoBufferWriteInfo = {
			.size = sizeof(VkDrawIndexedIndirectCommand),
			.data = &drawInfo,
			.offset = (material->shader == SHADER_SHADED ? shadedCount : i - shadedCount) *
					  sizeof(VkDrawIndexedIndirectCommand),
			.synchronizationType = LUNA_BUFFER_SYNCHRONIZATION_TYPE_INDIRECT_BUFFER,
		};
		VulkanTest(lunaWriteDataToBuffer(device,
										 commandBuffer,
										 material->shader == SHADER_SHADED ? buffer->shadedDrawInfo
																		   : buffer->unshadedDrawInfo,
										 &drawInfoBufferWriteInfo),
				   "Failed to write draw info to buffer!");

		shadedCount += material->shader == SHADER_SHADED ? 1 : 0;
	}

	VK_EndDebugLabel();
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
