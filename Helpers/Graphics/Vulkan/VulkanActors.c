//
// Created by NBT22 on 5/8/25.
//

#include "VulkanActors.h"
#include <assert.h>
#include <luna/luna.h>
#include "../../Core/Error.h"
#include "../RenderingHelpers.h"
#include "VulkanHelpers.h"
#include "VulkanResources.h"

static size_t loadedActorCount;
static List loadedSkins;
static List lodIdsLoadedForDraw;
static List loadedLodIds;
static List shadedMaterialCounts;
static List unshadedMaterialCounts;
static List materialCounts;
static List shadedMaterialIds;
static List unshadedMaterialIds;
static uint32_t indexOffset;
static int32_t vertexOffset;
static size_t indexDataOffset;
static size_t vertexDataOffset;

void LoadMaterial(List *materialIds,
				  List *materialCounts,
				  const size_t lodMaterialId,
				  BufferRegion *drawInfoBufferRegion,
				  const uint32_t indexCount)
{
	size_t index = ListFind(*materialIds, (void *)lodMaterialId);
	if (index == -1)
	{
		ListAdd(materialIds, (void *)lodMaterialId);
		ListAdd(materialCounts, (void *)1);
		if (!drawInfoBufferRegion)
		{
			return;
		}
		index = materialIds->length - 1;
	} else
	{
		ListGet(*materialCounts, index)++;
		if (!drawInfoBufferRegion)
		{
			return;
		}
	}

	VkDrawIndexedIndirectCommand *drawInfo = drawInfoBufferRegion->data;
	drawInfo[index].indexCount += indexCount;
	drawInfo[index].firstIndex += indexOffset;
	drawInfo[index].vertexOffset += vertexOffset;
	indexOffset += indexCount;

	drawInfoBufferRegion->bytesUsed += sizeof(VkDrawIndexedIndirectCommand);
}

void LoadLodForDraw(const Actor *actor, const ModelLod *lod)
{
	List *skins = calloc(1, sizeof(List));
	CheckAlloc(skins);
	ListAdd(&loadedSkins, skins);

	ListAdd((List *)ListGet(loadedSkins, loadedSkins.length - 1), (void *)(size_t)actor->currentSkinIndex);
	for (uint8_t j = 0; j < actor->actorModel->materialCount; j++)
	{
		const size_t indexSize = sizeof(uint32_t) * lod->indexCount[j];
		memcpy(buffers.actorModels.indices.data + indexDataOffset, (void *)lod->indexData[j], indexSize);
		indexDataOffset += indexSize;

		const Material material = actor->actorModel->skins[actor->currentSkinIndex][j];
		const uint32_t indexCount = lod->indexCount[j];
		const size_t lodMaterialId = (lod->id << 32) | material.id;
		switch (material.shader)
		{
			case SHADER_SHADED:
				LoadMaterial(&shadedMaterialIds,
							 &shadedMaterialCounts,
							 lodMaterialId,
							 &buffers.actorModels.shadedDrawInfo,
							 indexCount);
				break;
			case SHADER_UNSHADED:
				LoadMaterial(&unshadedMaterialIds,
							 &unshadedMaterialCounts,
							 lodMaterialId,
							 &buffers.actorModels.unshadedDrawInfo,
							 indexCount);
				break;
			default:
				assert(false && "Invalid material shader!");
		}
	}
}

void LoadLod(const Actor *actor, const uint32_t lodIndex)
{
	const ModelLod *lod = actor->actorModel->lods[lodIndex];
	assert(lod);
	if (lodIndex != actor->currentLod)
	{
		const size_t lodIdIndex = ListFind(loadedLodIds, (void *)lod->id);
		if (lodIdIndex == -1)
		{
			const size_t vertexSize = sizeof(ModelVertex) * lod->vertexCount;
			memcpy(buffers.actorModels.vertices.data + vertexDataOffset, (void *)lod->vertexData, vertexSize);
			vertexDataOffset += vertexSize;
			ListAdd(&loadedLodIds, (void *)lod->id);

			for (uint8_t j = 0; j < actor->actorModel->materialCount; j++)
			{
				const size_t indexSize = sizeof(uint32_t) * lod->indexCount[j];
				memcpy(buffers.actorModels.indices.data + indexDataOffset, (void *)lod->indexData[j], indexSize);
				indexDataOffset += indexSize;
			}

			buffers.actorModels.vertices.bytesUsed += sizeof(ModelVertex) * lod->vertexCount;
			buffers.actorModels.indices.bytesUsed += sizeof(uint32_t) * lod->totalIndexCount;
			vertexOffset += (int32_t)lod->vertexCount;
			indexOffset += lod->totalIndexCount;
		}
	} else
	{
		const size_t lodIdIndex = ListFind(lodIdsLoadedForDraw, (void *)lod->id);
		if (lodIdIndex == -1 ||
			ListFind(*(List *)ListGet(loadedSkins, lodIdIndex), (void *)(size_t)actor->currentSkinIndex) == -1)
		{
			const size_t vertexSize = sizeof(ModelVertex) * lod->vertexCount;
			memcpy(buffers.actorModels.vertices.data + vertexDataOffset, (void *)lod->vertexData, vertexSize);
			vertexDataOffset += vertexSize;
			ListAdd(&lodIdsLoadedForDraw, (void *)lod->id);
			LoadLodForDraw(actor, lod);
			buffers.actorModels.vertices.bytesUsed += sizeof(ModelVertex) * lod->vertexCount;
			buffers.actorModels.indices.bytesUsed += sizeof(uint32_t) * lod->totalIndexCount;
			vertexOffset += (int32_t)lod->vertexCount;
		} else
		{
			for (uint8_t j = 0; j < actor->actorModel->materialCount; j++)
			{
				const Material material = actor->actorModel->skins[actor->currentSkinIndex][j];
				const size_t lodMaterialId = (lod->id << 32) | material.id;
				switch (material.shader)
				{
					case SHADER_SHADED:
						LoadMaterial(&shadedMaterialIds, &shadedMaterialCounts, lodMaterialId, NULL, 0);
						break;
					case SHADER_UNSHADED:
						LoadMaterial(&unshadedMaterialIds, &unshadedMaterialCounts, lodMaterialId, NULL, 0);
						break;
					default:
						assert(false && "Invalid material shader!");
				}
			}
		}
		buffers.actorModels.instanceData.bytesUsed += sizeof(ModelInstanceData) * actor->actorModel->materialCount;
	}
}

VkResult PreSizeActorBuffers(const List *actors)
{
	for (size_t i = 0; i < loadedActorCount; i++)
	{
		assert(loadedActorCount == actors->length);
		const Actor *actor = ListGet(*actors, i);
		if (!actor->actorModel)
		{
			if (!actor->actorWall)
			{
				continue;
			}
			buffers.actorWalls.count++;
		} else
		{
			const ModelDefinition *model = actor->actorModel;
			for (uint8_t j = 0; j < model->lodCount; j++)
			{
				const ModelLod *lod = model->lods[j];
				buffers.actorModels.vertices.bytesUsed += sizeof(ModelVertex) * lod->vertexCount;
				buffers.actorModels.indices.bytesUsed += sizeof(uint32_t) * lod->totalIndexCount;
			}
			for (uint8_t j = 0; j < model->materialCount; j++)
			{
				switch (model->skins[actor->currentSkinIndex][j].shader)
				{
					case SHADER_SHADED:
						buffers.actorModels.shadedDrawInfo.bytesUsed += sizeof(VkDrawIndexedIndirectCommand);
						break;
					case SHADER_UNSHADED:
						buffers.actorModels.unshadedDrawInfo.bytesUsed += sizeof(VkDrawIndexedIndirectCommand);
						break;
					default:
						assert(false && "Invalid material shader!");
				}
			}
			buffers.actorModels.instanceData.bytesUsed += sizeof(ModelInstanceData) * model->materialCount;
		}
	}
	buffers.actorWalls.vertices.bytesUsed = sizeof(ActorWallVertex) * 4 * buffers.actorWalls.count;
	buffers.actorWalls.indices.bytesUsed = sizeof(uint32_t) * 6 * buffers.actorWalls.count;
	buffers.actorWalls.instanceData.bytesUsed = sizeof(ActorWallInstanceData) * buffers.actorWalls.count;
	buffers.actorWalls.drawInfo.bytesUsed = sizeof(VkDrawIndexedIndirectCommand) * buffers.actorWalls.count;
	VulkanTestReturnResult(ResizeActorWallBuffers(), "Failed to resize wall actor buffers!");
	VulkanTestReturnResult(ResizeActorModelBuffers(), "Failed to resize model actor buffers!");
	buffers.actorWalls.count = 0;
	buffers.actorModels.vertices.bytesUsed = 0;
	buffers.actorModels.indices.bytesUsed = 0;
	buffers.actorModels.instanceData.bytesUsed = 0;
	buffers.actorModels.shadedDrawInfo.bytesUsed = 0;
	buffers.actorModels.unshadedDrawInfo.bytesUsed = 0;

	return VK_SUCCESS;
}

VkResult InitActors(const List *actors)
{
	assert(actors);
	ListFreeOnlyContents(loadedSkins);
	ListClear(&loadedSkins);
	ListClear(&loadedLodIds);
	ListClear(&lodIdsLoadedForDraw);
	ListClear(&shadedMaterialCounts);
	ListClear(&unshadedMaterialCounts);
	ListClear(&materialCounts);
	ListClear(&shadedMaterialIds);
	ListClear(&unshadedMaterialIds);
	indexOffset = 0;
	vertexOffset = 0;
	indexDataOffset = 0;
	vertexDataOffset = 0;
	memset(buffers.actorModels.shadedDrawInfo.data, 0, buffers.actorModels.shadedDrawInfo.bytesUsed);
	memset(buffers.actorModels.unshadedDrawInfo.data, 0, buffers.actorModels.unshadedDrawInfo.bytesUsed);
	memset(buffers.actorWalls.drawInfo.data, 0, buffers.actorWalls.drawInfo.bytesUsed);
	buffers.actorWalls.count = 0;
	buffers.actorModels.vertices.bytesUsed = 0;
	buffers.actorModels.indices.bytesUsed = 0;
	buffers.actorModels.instanceData.bytesUsed = 0;
	buffers.actorModels.shadedDrawInfo.bytesUsed = 0;
	buffers.actorModels.unshadedDrawInfo.bytesUsed = 0;
	ListLock(*actors);
	loadedActorCount = actors->length;

	VulkanTestReturnResult(PreSizeActorBuffers(actors), "Failed to pre-size actor buffers!");

	VkDrawIndexedIndirectCommand *shadedActorModelsDrawInfo = buffers.actorModels.shadedDrawInfo.data;
	VkDrawIndexedIndirectCommand *unshadedActorModelsDrawInfo = buffers.actorModels.unshadedDrawInfo.data;
	VkDrawIndexedIndirectCommand *actorWallsDrawInfo = buffers.actorWalls.drawInfo.data;
	for (size_t i = 0; i < loadedActorCount; i++)
	{
		assert(loadedActorCount == actors->length);
		const Actor *actor = ListGet(*actors, i);
		if (!actor->actorModel)
		{
			if (!actor->actorWall)
			{
				continue;
			}
			actorWallsDrawInfo[buffers.actorWalls.count].indexCount = 6;
			actorWallsDrawInfo[buffers.actorWalls.count].instanceCount = 1;
			actorWallsDrawInfo[buffers.actorWalls.count].firstIndex = buffers.actorWalls.count * 6;
			actorWallsDrawInfo[buffers.actorWalls.count].vertexOffset = 0;
			actorWallsDrawInfo[buffers.actorWalls.count].firstInstance = buffers.actorWalls.count;
			buffers.actorWalls.count++;
			assert(sizeof(VkDrawIndexedIndirectCommand) * buffers.actorWalls.count <=
				   buffers.actorWalls.drawInfo.allocatedSize);
		} else
		{
			for (uint8_t j = 0; j < actor->actorModel->lodCount; j++)
			{
				LoadLod(actor, j);
			}
		}
	}
	assert(buffers.actorWalls.vertices.bytesUsed <= sizeof(ActorWallVertex) * 4 * buffers.actorWalls.count);
	assert(buffers.actorWalls.indices.bytesUsed <= sizeof(uint32_t) * 6 * buffers.actorWalls.count);
	assert(buffers.actorWalls.instanceData.bytesUsed <= sizeof(ActorWallInstanceData) * buffers.actorWalls.count);
	assert(buffers.actorWalls.drawInfo.bytesUsed <= sizeof(VkDrawIndexedIndirectCommand) * buffers.actorWalls.count);
	assert(buffers.actorModels.vertices.bytesUsed <= buffers.actorModels.vertices.allocatedSize);
	assert(buffers.actorModels.indices.bytesUsed <= buffers.actorModels.indices.allocatedSize);
	assert(buffers.actorModels.instanceData.bytesUsed <= buffers.actorModels.instanceData.allocatedSize);
	assert(buffers.actorModels.shadedDrawInfo.bytesUsed <= buffers.actorModels.shadedDrawInfo.allocatedSize);
	assert(buffers.actorModels.unshadedDrawInfo.bytesUsed <= buffers.actorModels.unshadedDrawInfo.allocatedSize);
	ListUnlock(*actors);

	size_t totalInstanceCount = 0;
	for (size_t i = 0; i < shadedMaterialCounts.length; i++)
	{
		const size_t instanceCount = (size_t)ListGet(shadedMaterialCounts, i);
		ListAdd(&materialCounts, (void *)instanceCount);
		shadedActorModelsDrawInfo[i].instanceCount = instanceCount;
		shadedActorModelsDrawInfo[i].firstInstance = totalInstanceCount;
		totalInstanceCount += instanceCount;
	}
	for (size_t i = 0; i < unshadedMaterialCounts.length; i++)
	{
		const size_t instanceCount = (size_t)ListGet(unshadedMaterialCounts, i);
		ListAdd(&materialCounts, (void *)instanceCount);
		unshadedActorModelsDrawInfo[i].instanceCount = instanceCount;
		unshadedActorModelsDrawInfo[i].firstInstance = totalInstanceCount;
		totalInstanceCount += instanceCount;
	}

	lunaWriteDataToBuffer(buffers.actorWalls.drawInfo.buffer,
						  buffers.actorWalls.drawInfo.data,
						  buffers.actorWalls.drawInfo.bytesUsed,
						  0);
	lunaWriteDataToBuffer(buffers.actorModels.vertices.buffer,
						  buffers.actorModels.vertices.data,
						  buffers.actorModels.vertices.bytesUsed,
						  0);
	lunaWriteDataToBuffer(buffers.actorModels.indices.buffer,
						  buffers.actorModels.indices.data,
						  buffers.actorModels.indices.bytesUsed,
						  0);
	lunaWriteDataToBuffer(buffers.actorModels.shadedDrawInfo.buffer,
						  buffers.actorModels.shadedDrawInfo.data,
						  buffers.actorModels.shadedDrawInfo.bytesUsed,
						  0);
	lunaWriteDataToBuffer(buffers.actorModels.unshadedDrawInfo.buffer,
						  buffers.actorModels.unshadedDrawInfo.data,
						  buffers.actorModels.unshadedDrawInfo.bytesUsed,
						  0);

	return VK_SUCCESS;
}

VkResult LoadActorWalls(const List *actors)
{
	assert(actors);
	if (__builtin_expect(loadedActorCount != actors->length, false))
	{
		VulkanTestReturnResult(InitActors(actors), "Failed to init actors!");
	}
	uint32_t wallCount = 0;
	ActorWallVertex *vertices = buffers.actorWalls.vertices.data;
	uint32_t *indices = buffers.actorWalls.indices.data;
	for (size_t i = 0; i < loadedActorCount; i++)
	{
		const Actor *actor = ListGet(*actors, i);
		if (!actor->actorWall || actor->actorModel != NULL)
		{
			continue;
		}
		const Wall *wall = actor->actorWall;
		const float halfHeight = wall->height / 2.0f;
		const vec2 startVertex = {actor->position.x + wall->a.x, actor->position.y + wall->a.y};
		const vec2 endVertex = {actor->position.x + wall->b.x, actor->position.y + wall->b.y};
		const vec2 startUV = {wall->uvOffset, 0};
		const vec2 endUV = {wall->uvScale * wall->length + wall->uvOffset, 1};

		vertices[4 * wallCount].x = startVertex[0];
		vertices[4 * wallCount].y = halfHeight + actor->yPosition;
		vertices[4 * wallCount].z = startVertex[1];
		vertices[4 * wallCount].u = startUV[0];
		vertices[4 * wallCount].v = startUV[1];

		vertices[4 * wallCount + 1].x = endVertex[0];
		vertices[4 * wallCount + 1].y = halfHeight + actor->yPosition;
		vertices[4 * wallCount + 1].z = endVertex[1];
		vertices[4 * wallCount + 1].u = endUV[0];
		vertices[4 * wallCount + 1].v = startUV[1];

		vertices[4 * wallCount + 2].x = endVertex[0];
		vertices[4 * wallCount + 2].y = -halfHeight + actor->yPosition;
		vertices[4 * wallCount + 2].z = endVertex[1];
		vertices[4 * wallCount + 2].u = endUV[0];
		vertices[4 * wallCount + 2].v = endUV[1];

		vertices[4 * wallCount + 3].x = startVertex[0];
		vertices[4 * wallCount + 3].y = -halfHeight + actor->yPosition;
		vertices[4 * wallCount + 3].z = startVertex[1];
		vertices[4 * wallCount + 3].u = startUV[0];
		vertices[4 * wallCount + 3].v = endUV[1];

		indices[6 * wallCount] = wallCount * 4;
		indices[6 * wallCount + 1] = wallCount * 4 + 1;
		indices[6 * wallCount + 2] = wallCount * 4 + 2;
		indices[6 * wallCount + 3] = wallCount * 4;
		indices[6 * wallCount + 4] = wallCount * 4 + 2;
		indices[6 * wallCount + 5] = wallCount * 4 + 3;

		wallCount++;
	}
	lunaWriteDataToBuffer(buffers.actorWalls.vertices.buffer,
						  buffers.actorWalls.vertices.data,
						  buffers.actorWalls.vertices.bytesUsed,
						  0);
	lunaWriteDataToBuffer(buffers.actorWalls.indices.buffer,
						  buffers.actorWalls.indices.data,
						  buffers.actorWalls.indices.bytesUsed,
						  0);

	return VK_SUCCESS;
}

VkResult UpdateActorInstanceData(const List *actors)
{
	assert(actors);
	uint32_t wallCount = 0;
	size_t offsets[materialCounts.length];
	if (materialCounts.length > 0)
	{
		size_t currentOffset = 0;
		offsets[0] = currentOffset;
		currentOffset += sizeof(ModelInstanceData) * (size_t)ListGet(materialCounts, 0);
		for (size_t i = 1; i < materialCounts.length; i++)
		{
			offsets[i] = currentOffset;
			currentOffset += sizeof(ModelInstanceData) * (size_t)ListGet(materialCounts, i);
		}
	}
	if (__builtin_expect(loadedActorCount != actors->length, false))
	{
		VulkanTestReturnResult(InitActors(actors), "Failed to init actors!");
	}
	ActorWallInstanceData *actorWallsInstanceData = buffers.actorWalls.instanceData.data;
	for (size_t i = 0; i < loadedActorCount; i++)
	{
		const Actor *actor = ListGet(*actors, i);
		if (!actor->actorWall && !actor->actorModel)
		{
			continue;
		}

		if (actor->actorModel)
		{
			mat4 transformMatrix = GLM_MAT4_IDENTITY_INIT;
			ActorTransformMatrix(actor, &transformMatrix);
			for (byte j = 0; j < actor->actorModel->materialCount; j++)
			{
				const Material material = actor->actorModel->skins[actor->currentSkinIndex][j];
				size_t index = 0;
				const size_t lodMaterialId = (actor->actorModel->lods[actor->currentLod]->id << 32) | material.id;
				switch (material.shader)
				{
					case SHADER_SHADED:
						index = ListFind(shadedMaterialIds, (void *)lodMaterialId);
						if (index == -1)
						{
							VulkanTestReturnResult(InitActors(actors), "Failed to init actors!");
							return VK_SUCCESS;
						}
						break;
					case SHADER_UNSHADED:
						index = ListFind(unshadedMaterialIds, (void *)lodMaterialId) + shadedMaterialIds.length;
						if (index == -1)
						{
							VulkanTestReturnResult(InitActors(actors), "Failed to init actors!");
							return VK_SUCCESS;
						}
						break;
					case SHADER_SKY:
					default:
						assert(false && "Invalid material shader!");
				}
				ModelInstanceData *offsetInstanceData = buffers.actorModels.instanceData.data + offsets[index];
				memcpy(offsetInstanceData->transform, transformMatrix, sizeof(mat4));
				offsetInstanceData->textureIndex = TextureIndex(material.texture);
				offsetInstanceData->color = material.color;
				offsets[index] += sizeof(ModelInstanceData);
			}
		} else if (actor->actorWall)
		{
			const Wall *wall = actor->actorWall;
			memcpy(actorWallsInstanceData[wallCount].transform, GLM_MAT4_IDENTITY, sizeof(mat4));
			actorWallsInstanceData[wallCount].textureIndex = TextureIndex(wall->tex);
			actorWallsInstanceData[wallCount].wallAngle = actor->actorWall->angle;

			wallCount++;
		}
	}
	lunaWriteDataToBuffer(buffers.actorWalls.instanceData.buffer,
						  buffers.actorWalls.instanceData.data,
						  buffers.actorWalls.instanceData.bytesUsed,
						  0);
	lunaWriteDataToBuffer(buffers.actorModels.instanceData.buffer,
						  buffers.actorModels.instanceData.data,
						  buffers.actorModels.instanceData.bytesUsed,
						  0);

	return VK_SUCCESS;
}

void DestroyActorMetadata()
{
	ListAndContentsFree(&loadedSkins, false);
	ListFree(&lodIdsLoadedForDraw, false);
	ListFree(&loadedLodIds, false);
	ListFree(&shadedMaterialCounts, false);
	ListFree(&unshadedMaterialCounts, false);
	ListFree(&materialCounts, false);
	ListFree(&shadedMaterialIds, false);
	ListFree(&unshadedMaterialIds, false);
}
