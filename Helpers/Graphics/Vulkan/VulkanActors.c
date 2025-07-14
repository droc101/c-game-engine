//
// Created by NBT22 on 5/8/25.
//

#include "VulkanActors.h"
#include <assert.h>
#include <luna/luna.h>
#include "../RenderingHelpers.h"
#include "VulkanHelpers.h"
#include "VulkanResources.h"

// TODO: cognitive complexity :(
VkResult InitActors(const List *actors)
{
	assert(actors);

	List loadedSkins = {0};
	List loadedLodIds = {0};
	List shadedMaterialCounts = {0};
	List unshadedMaterialCounts = {0};
	uint32_t indexOffset = 0;
	int32_t vertexOffset = 0;
	size_t indexDataOffset = 0;
	size_t shadedVertexDataOffset = 0;
	VkDrawIndexedIndirectCommand *actorWallsDrawInfo = buffers.actorWalls.drawInfo.data;
	VkDrawIndexedIndirectCommand *shadedActorModelsDrawInfo = buffers.actorModels.shadedDrawInfo.data;
	VkDrawIndexedIndirectCommand *unshadedActorModelsDrawInfo = buffers.actorModels.unshadedDrawInfo.data;
	memset(actorWallsDrawInfo, 0, buffers.actorWalls.drawInfo.bytesUsed);
	memset(shadedActorModelsDrawInfo, 0, buffers.actorModels.shadedDrawInfo.bytesUsed);
	memset(unshadedActorModelsDrawInfo, 0, buffers.actorModels.unshadedDrawInfo.bytesUsed);
	ListClear(&buffers.actorModels.materialCounts);
	ListClear(&buffers.actorModels.shadedMaterialIds);
	ListClear(&buffers.actorModels.unshadedMaterialIds);
	buffers.actorWalls.count = 0;
	buffers.actorModels.vertices.bytesUsed = 0;
	buffers.actorModels.indices.bytesUsed = 0;
	buffers.actorModels.instanceData.bytesUsed = 0;
	buffers.actorModels.shadedDrawInfo.bytesUsed = 0;
	buffers.actorModels.unshadedDrawInfo.bytesUsed = 0;
	ListLock(*actors);
	loadedActors = actors->length;
	for (size_t i = 0; i < loadedActors; i++)
	{
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
		} else
		{
			const ModelLod *lod = actor->actorModel->lods[actor->currentLod];
			assert(lod);
			size_t lodIndex = ListFind(loadedLodIds, (void *)lod->id);
			if (lodIndex == -1 ||
				ListFind(*(List *)ListGet(loadedSkins, lodIndex), (void *)(size_t)actor->currentSkinIndex) == -1)
			{
				lodIndex = loadedLodIds.length;
				ListAdd(&loadedLodIds, (void *)lod->id);
				List *skins = calloc(1, sizeof(List));
				ListAdd(&loadedSkins, skins);
				const size_t shadedVertexSize = sizeof(ModelVertex) * lod->vertexCount;
				memcpy(buffers.actorModels.vertices.data + shadedVertexDataOffset,
					   (void *)lod->vertexData,
					   shadedVertexSize);
				shadedVertexDataOffset += shadedVertexSize;

				ListAdd((List *)ListGet(loadedSkins, lodIndex), (void *)(size_t)actor->currentSkinIndex);
				for (uint8_t j = 0; j < actor->actorModel->materialCount; j++)
				{
					const size_t indexSize = sizeof(uint32_t) * lod->indexCount[j];
					memcpy(buffers.actorModels.indices.data + indexDataOffset, (void *)lod->indexData[j], indexSize);
					indexDataOffset += indexSize;

					const Material material = actor->actorModel->skins[actor->currentSkinIndex][j];
					const uint32_t indexCount = lod->indexCount[j];
					size_t index;
					switch (material.shader)
					{
						case SHADER_SHADED:
							index = ListFind(buffers.actorModels.shadedMaterialIds, (void *)material.id);
							if (index == -1)
							{
								index = buffers.actorModels.shadedMaterialIds.length;
								ListAdd(&buffers.actorModels.shadedMaterialIds, (void *)material.id);
								ListAdd(&shadedMaterialCounts, (void *)1);
							} else
							{
								ListGet(shadedMaterialCounts, index)++;
							}

							shadedActorModelsDrawInfo[index].indexCount += indexCount;
							shadedActorModelsDrawInfo[index].firstIndex += indexOffset;
							shadedActorModelsDrawInfo[index].vertexOffset += vertexOffset;
							indexOffset += indexCount;

							buffers.actorModels.vertices.bytesUsed += sizeof(ModelVertex) * lod->vertexCount;
							buffers.actorModels.indices.bytesUsed += sizeof(uint32_t) * indexCount;
							buffers.actorModels.shadedDrawInfo.bytesUsed += sizeof(VkDrawIndexedIndirectCommand);
							break;
						case SHADER_UNSHADED:
							index = ListFind(buffers.actorModels.unshadedMaterialIds, (void *)material.id);
							if (index == -1)
							{
								index = buffers.actorModels.unshadedMaterialIds.length;
								ListAdd(&buffers.actorModels.unshadedMaterialIds, (void *)material.id);
								ListAdd(&unshadedMaterialCounts, (void *)1);
							} else
							{
								ListGet(unshadedMaterialCounts, index)++;
							}

							unshadedActorModelsDrawInfo[index].indexCount += indexCount;
							unshadedActorModelsDrawInfo[index].firstIndex += indexOffset;
							unshadedActorModelsDrawInfo[index].vertexOffset += vertexOffset;
							indexOffset += indexCount;

							buffers.actorModels.vertices.bytesUsed += sizeof(ModelVertex) * lod->vertexCount;
							buffers.actorModels.indices.bytesUsed += sizeof(uint32_t) * indexCount;
							buffers.actorModels.unshadedDrawInfo.bytesUsed += sizeof(VkDrawIndexedIndirectCommand);
							break;
						case SHADER_SKY:
						default:
							assert(false && "Invalid material shader!");
					}
				}
				vertexOffset += (int32_t)lod->vertexCount;
			} else
			{
				for (uint8_t j = 0; j < actor->actorModel->materialCount; j++)
				{
					const Material material = actor->actorModel->skins[actor->currentSkinIndex][j];
					size_t index;
					switch (material.shader)
					{
						case SHADER_SHADED:
							index = ListFind(buffers.actorModels.shadedMaterialIds, (void *)material.id);
							if (index == -1)
							{
								ListAdd(&buffers.actorModels.shadedMaterialIds, (void *)material.id);
								ListAdd(&shadedMaterialCounts, (void *)1);
							} else
							{
								ListGet(shadedMaterialCounts, index)++;
							}
							break;
						case SHADER_UNSHADED:
							index = ListFind(buffers.actorModels.unshadedMaterialIds, (void *)material.id);
							if (index == -1)
							{
								ListAdd(&buffers.actorModels.unshadedMaterialIds, (void *)material.id);
								ListAdd(&unshadedMaterialCounts, (void *)1);
							} else
							{
								ListGet(unshadedMaterialCounts, index)++;
							}
							break;
						case SHADER_SKY:
						default:
							assert(false && "Invalid material shader!");
					}
				}
			}
			buffers.actorModels.instanceData.bytesUsed += sizeof(ModelInstanceData) * actor->actorModel->materialCount;
		}
	}
	buffers.actorWalls.vertices.bytesUsed = sizeof(ActorVertex) * 4 * buffers.actorWalls.count;
	buffers.actorWalls.indices.bytesUsed = sizeof(uint32_t) * 6 * buffers.actorWalls.count;
	buffers.actorWalls.instanceData.bytesUsed = sizeof(ActorWallInstanceData) * buffers.actorWalls.count;
	buffers.actorWalls.drawInfo.bytesUsed = sizeof(VkDrawIndexedIndirectCommand) * buffers.actorWalls.count;

	VulkanTestReturnResult(ResizeActorWallBuffers(), "Failed to resize wall actor buffers!");
	VulkanTestReturnResult(ResizeActorModelBuffers(), "Failed to resize model actor buffers!");
	assert(loadedActors == actors->length);
	ListUnlock(*actors);

	size_t totalInstanceCount = 0;
	for (size_t i = 0; i < shadedMaterialCounts.length; i++)
	{
		const size_t instanceCount = (size_t)ListGet(shadedMaterialCounts, i);
		ListAdd(&buffers.actorModels.materialCounts, (void *)instanceCount);
		shadedActorModelsDrawInfo[i].instanceCount = instanceCount;
		shadedActorModelsDrawInfo[i].firstInstance = totalInstanceCount;
		totalInstanceCount += instanceCount;
	}
	for (size_t i = 0; i < unshadedMaterialCounts.length; i++)
	{
		const size_t instanceCount = (size_t)ListGet(unshadedMaterialCounts, i);
		ListAdd(&buffers.actorModels.materialCounts, (void *)instanceCount);
		unshadedActorModelsDrawInfo[i].instanceCount = instanceCount;
		unshadedActorModelsDrawInfo[i].firstInstance = totalInstanceCount;
		totalInstanceCount += instanceCount;
	}

	for (size_t i = 0; i < loadedSkins.length; i++)
	{
		ListFree(ListGet(loadedSkins, i), true);
	}
	ListFree(&loadedSkins, false);
	ListFree(&loadedLodIds, false);
	ListFree(&shadedMaterialCounts, false);
	ListFree(&unshadedMaterialCounts, false);

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
	if (__builtin_expect(loadedActors != actors->length, false))
	{
		VulkanTestReturnResult(InitActors(actors), "Failed to init actors!");
	}
	uint32_t wallCount = 0;
	ActorVertex *vertices = buffers.actorWalls.vertices.data;
	uint32_t *indices = buffers.actorWalls.indices.data;
	for (size_t i = 0; i < loadedActors; i++)
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
	size_t offsets[buffers.actorModels.materialCounts.length];
	if (buffers.actorModels.materialCounts.length > 0)
	{
		size_t currentOffset = 0;
		offsets[0] = currentOffset;
		currentOffset += sizeof(ModelInstanceData) * (size_t)ListGet(buffers.actorModels.materialCounts, 0);
		for (size_t i = 1; i < buffers.actorModels.materialCounts.length; i++)
		{
			offsets[i] = currentOffset;
			currentOffset += sizeof(ModelInstanceData) * (size_t)ListGet(buffers.actorModels.materialCounts, i);
		}
	}
	if (__builtin_expect(loadedActors != actors->length, false))
	{
		VulkanTestReturnResult(InitActors(actors), "Failed to init actors!");
	}
	ActorWallInstanceData *actorWallsInstanceData = buffers.actorWalls.instanceData.data;
	for (size_t i = 0; i < loadedActors; i++)
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
				switch (material.shader)
				{
					case SHADER_SHADED:
						index = ListFind(buffers.actorModels.shadedMaterialIds, (void *)material.id);
						assert(index != -1);
						break;
					case SHADER_UNSHADED:
						assert(ListFind(buffers.actorModels.unshadedMaterialIds, (void *)material.id) != -1);
						index = ListFind(buffers.actorModels.unshadedMaterialIds, (void *)material.id) +
								buffers.actorModels.shadedMaterialIds.length;
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
