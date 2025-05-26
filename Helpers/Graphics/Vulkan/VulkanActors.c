//
// Created by NBT22 on 5/8/25.
//

#include "VulkanActors.h"
#include <luna/luna.h>
#include "../RenderingHelpers.h"
#include "VulkanHelpers.h"
#include "VulkanResources.h"

VkResult InitActors(const Level *level)
{
	if (loadedActors == level->actors.length)
	{
		return VK_SUCCESS;
	}

	ListClear(&buffers.modelActors.loadedModelIds);
	ListClear(&buffers.modelActors.modelCounts);
	buffers.shadows.objectCount = 0;
	free(buffers.wallActors.vertices.data);
	free(buffers.wallActors.indices.data);
	free(buffers.wallActors.instanceData.data);
	free(buffers.wallActors.drawInfo.data);
	free(buffers.modelActors.vertices.data);
	free(buffers.modelActors.indices.data);
	free(buffers.modelActors.instanceData.data);
	free(buffers.modelActors.drawInfo.data);
	memset(&buffers.wallActors, 0, sizeof(WallActorsBuffer));
	memset(&buffers.modelActors, 0, sizeof(ModelActorsBuffer));
	ListLock(level->actors);
	loadedActors = level->actors.length;
	for (size_t i = 0; i < loadedActors; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorModel)
		{
			if (!actor->actorWall)
			{
				continue;
			}
			buffers.wallActors.count++;
		} else
		{
			size_t index = ListFind(buffers.modelActors.loadedModelIds, (void *)actor->actorModel->id);
			if (index == -1)
			{
				index = buffers.modelActors.loadedModelIds.length;
				ListAdd(&buffers.modelActors.loadedModelIds, (void *)actor->actorModel->id);
				buffers.modelActors.vertices.bytesUsed += sizeof(ActorVertex) * actor->actorModel->vertexCount;
				buffers.modelActors.indices.bytesUsed += sizeof(uint32_t) * actor->actorModel->indexCount;
				buffers.modelActors.drawInfo.bytesUsed += sizeof(VkDrawIndexedIndirectCommand);
			}
			buffers.modelActors.instanceData.bytesUsed += sizeof(ActorInstanceData);
			if (index < buffers.modelActors.modelCounts.length)
			{
				buffers.modelActors.modelCounts.data[index]++;
			} else
			{
				ListAdd(&buffers.modelActors.modelCounts, (void *)1);
			}
		}
		if (actor->showShadow)
		{
			buffers.shadows.objectCount++;
		}
	}
	buffers.shadows.vertices.bytesUsed = sizeof(WallVertex) * 4 * buffers.shadows.objectCount;
	buffers.shadows.indices.bytesUsed = sizeof(uint32_t) * 6 * buffers.shadows.objectCount;
	buffers.wallActors.vertices.bytesUsed = sizeof(ActorVertex) * 4 * buffers.wallActors.count;
	buffers.wallActors.indices.bytesUsed = sizeof(uint32_t) * 6 * buffers.wallActors.count;
	buffers.wallActors.instanceData.bytesUsed = sizeof(ActorInstanceData) * buffers.wallActors.count;
	buffers.wallActors.drawInfo.bytesUsed = sizeof(VkDrawIndexedIndirectCommand) * buffers.wallActors.count;

	VulkanTestReturnResult(ResizeShadowBuffers(), "Failed to resize shadow buffers!");
	VulkanTestReturnResult(ResizeWallActorBuffers(), "Failed to resize wall actor buffers!");
	VulkanTestReturnResult(ResizeModelActorBuffers(), "Failed to resize model actor buffers!");
	LoadModelActors(level);
	LoadActorDrawInfo(level);
	ListUnlock(level->actors);
	return VK_SUCCESS;
}

void LoadModelActors(const Level *level)
{
	size_t vertexOffset = 0;
	size_t indexOffset = 0;
	ListClear(&buffers.modelActors.loadedModelIds);
	ListLock(level->actors);
	for (size_t i = 0; i < loadedActors; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorModel)
		{
			continue;
		}
		if (ListFind(buffers.modelActors.loadedModelIds, (void *)actor->actorModel->id) == -1)
		{
			ListAdd(&buffers.modelActors.loadedModelIds, (void *)actor->actorModel->id);
			const size_t vertexSize = sizeof(ActorVertex) * actor->actorModel->vertexCount;
			const size_t indexSize = sizeof(uint32_t) * actor->actorModel->indexCount;
			memcpy(buffers.modelActors.vertices.data + vertexOffset, actor->actorModel->vertexData, vertexSize);
			memcpy(buffers.modelActors.indices.data + indexOffset, actor->actorModel->indexData, indexSize);
			vertexOffset += vertexSize;
			indexOffset += indexSize;
		}
	}
	ListUnlock(level->actors);
	lunaWriteDataToBuffer(buffers.modelActors.vertices.buffer,
						  buffers.modelActors.vertices.data,
						  buffers.modelActors.vertices.bytesUsed);
	lunaWriteDataToBuffer(buffers.modelActors.indices.buffer,
						  buffers.modelActors.indices.data,
						  buffers.modelActors.indices.bytesUsed);
}

void LoadActorDrawInfo(const Level *level)
{
	VkDrawIndexedIndirectCommand *modelActorsDrawInfo = buffers.modelActors.drawInfo.data;
	VkDrawIndexedIndirectCommand *wallActorsDrawInfo = buffers.wallActors.drawInfo.data;
	uint32_t wallCount = 0;
	if (buffers.modelActors.loadedModelIds.length > 0)
	{
		modelActorsDrawInfo[0].indexCount = GetModelFromId((size_t)ListGet(buffers.modelActors.loadedModelIds, 0))
													->indexCount;
		modelActorsDrawInfo[0].instanceCount = (size_t)ListGet(buffers.modelActors.modelCounts, 0);
		size_t modelCount = (size_t)ListGet(buffers.modelActors.modelCounts, 0);
		for (size_t i = 1; i < buffers.modelActors.loadedModelIds.length; i++)
		{
			const Model *previousModel = GetModelFromId((size_t)ListGet(buffers.modelActors.loadedModelIds, i - 1));
			modelActorsDrawInfo[i].indexCount = GetModelFromId((size_t)ListGet(buffers.modelActors.loadedModelIds, i))
														->indexCount;
			modelActorsDrawInfo[i].instanceCount = (size_t)ListGet(buffers.modelActors.modelCounts, i);
			modelActorsDrawInfo[i].firstIndex = previousModel->indexCount;
			modelActorsDrawInfo[i].vertexOffset = (int32_t)previousModel->vertexCount;
			modelActorsDrawInfo[i].firstInstance = modelCount;
			modelCount += (size_t)ListGet(buffers.modelActors.modelCounts, i);
		}
	}
	ListLock(level->actors);
	for (size_t i = 0; i < loadedActors; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorWall || actor->actorModel)
		{
			continue;
		}
		if (actor->actorWall)
		{
			wallActorsDrawInfo[wallCount].indexCount = 6;
			wallActorsDrawInfo[wallCount].instanceCount = 1;
			wallActorsDrawInfo[wallCount].firstIndex = wallCount * 6;
			wallActorsDrawInfo[wallCount].vertexOffset = 0;
			wallActorsDrawInfo[wallCount].firstInstance = wallCount;
			wallCount++;
		}
	}
	ListUnlock(level->actors);
	lunaWriteDataToBuffer(buffers.wallActors.drawInfo.buffer,
						  buffers.wallActors.drawInfo.data,
						  buffers.wallActors.drawInfo.bytesUsed);
	lunaWriteDataToBuffer(buffers.modelActors.drawInfo.buffer,
						  buffers.modelActors.drawInfo.data,
						  buffers.modelActors.drawInfo.bytesUsed);
}

VkResult LoadWallActors(const Level *level)
{
	if (__builtin_expect(loadedActors != level->actors.length, false))
	{
		VulkanTestReturnResult(InitActors(level), "Failed to init actors!");
	}
	uint32_t wallCount = 0;
	ActorVertex *vertices = buffers.wallActors.vertices.data;
	uint32_t *indices = buffers.wallActors.indices.data;
	ListLock(level->actors);
	for (size_t i = 0; i < loadedActors; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
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
		vertices[4 * wallCount].nz = NAN;

		vertices[4 * wallCount + 1].x = endVertex[0];
		vertices[4 * wallCount + 1].y = halfHeight + actor->yPosition;
		vertices[4 * wallCount + 1].z = endVertex[1];
		vertices[4 * wallCount + 1].u = endUV[0];
		vertices[4 * wallCount + 1].v = startUV[1];
		vertices[4 * wallCount + 1].nz = NAN;

		vertices[4 * wallCount + 2].x = endVertex[0];
		vertices[4 * wallCount + 2].y = -halfHeight + actor->yPosition;
		vertices[4 * wallCount + 2].z = endVertex[1];
		vertices[4 * wallCount + 2].u = endUV[0];
		vertices[4 * wallCount + 2].v = endUV[1];
		vertices[4 * wallCount + 2].nz = NAN;

		vertices[4 * wallCount + 3].x = startVertex[0];
		vertices[4 * wallCount + 3].y = -halfHeight + actor->yPosition;
		vertices[4 * wallCount + 3].z = startVertex[1];
		vertices[4 * wallCount + 3].u = startUV[0];
		vertices[4 * wallCount + 3].v = endUV[1];
		vertices[4 * wallCount + 3].nz = NAN;

		indices[6 * wallCount] = wallCount * 4;
		indices[6 * wallCount + 1] = wallCount * 4 + 1;
		indices[6 * wallCount + 2] = wallCount * 4 + 2;
		indices[6 * wallCount + 3] = wallCount * 4;
		indices[6 * wallCount + 4] = wallCount * 4 + 2;
		indices[6 * wallCount + 5] = wallCount * 4 + 3;

		wallCount++;
	}
	ListUnlock(level->actors);
	lunaWriteDataToBuffer(buffers.wallActors.vertices.buffer,
						  buffers.wallActors.vertices.data,
						  buffers.wallActors.vertices.bytesUsed);
	lunaWriteDataToBuffer(buffers.wallActors.indices.buffer,
						  buffers.wallActors.indices.data,
						  buffers.wallActors.indices.bytesUsed);

	return VK_SUCCESS;
}

VkResult UpdateActorInstanceDataAndShadows(const Level *level)
{
	uint32_t wallCount = 0;
	uint32_t shadowCount = 0;
	uint16_t modelCounts[buffers.modelActors.loadedModelIds.length] = {};
	uint32_t offsets[buffers.modelActors.loadedModelIds.length + 1] = {};
	for (size_t i = 1; i <= buffers.modelActors.loadedModelIds.length; i++)
	{
		offsets[i] = offsets[i - 1] +
					 (size_t)ListGet(buffers.modelActors.modelCounts, i - 1) * sizeof(ActorInstanceData);
	}
	ListLock(level->actors);
	if (__builtin_expect(loadedActors != level->actors.length, false))
	{
		VulkanTestReturnResult(InitActors(level), "Failed to init actors!");
	}
	ActorInstanceData *wallActorInstanceData = buffers.wallActors.instanceData.data;
	ShadowVertex *shadowVertices = buffers.shadows.vertices.data;
	uint32_t *shadowIndices = buffers.shadows.indices.data;
	for (size_t i = 0; i < loadedActors; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorWall && !actor->actorModel)
		{
			continue;
		}

		if (actor->actorModel)
		{
			mat4 transformMatrix = GLM_MAT4_IDENTITY_INIT;
			ActorTransformMatrix(actor, &transformMatrix);
			const size_t index = ListFind(buffers.modelActors.loadedModelIds, (void *)actor->actorModel->id);
			ActorInstanceData *offsetInstanceData = buffers.modelActors.instanceData.data + offsets[index];
			memcpy(offsetInstanceData[modelCounts[index]].transform, transformMatrix, sizeof(mat4));
			offsetInstanceData[modelCounts[index]].textureIndex = TextureIndex(actor->actorModelTexture);

			modelCounts[index]++;
		} else if (actor->actorWall)
		{
			const Wall *wall = actor->actorWall;
			memcpy(wallActorInstanceData[wallCount].transform, GLM_MAT4_IDENTITY, sizeof(mat4));
			wallActorInstanceData[wallCount].textureIndex = TextureIndex(wall->tex);
			wallActorInstanceData[wallCount].wallAngle = actor->actorWall->angle;

			wallCount++;
		}
		if (actor->showShadow)
		{
			shadowVertices[4 * shadowCount].x = actor->position.x - 0.5f * actor->shadowSize;
			shadowVertices[4 * shadowCount].y = -0.49f;
			shadowVertices[4 * shadowCount].z = actor->position.y - 0.5f * actor->shadowSize;

			shadowVertices[4 * shadowCount + 1].x = actor->position.x + 0.5f * actor->shadowSize;
			shadowVertices[4 * shadowCount + 1].y = -0.49f;
			shadowVertices[4 * shadowCount + 1].z = actor->position.y - 0.5f * actor->shadowSize;

			shadowVertices[4 * shadowCount + 2].x = actor->position.x + 0.5f * actor->shadowSize;
			shadowVertices[4 * shadowCount + 2].y = -0.49f;
			shadowVertices[4 * shadowCount + 2].z = actor->position.y + 0.5f * actor->shadowSize;

			shadowVertices[4 * shadowCount + 3].x = actor->position.x - 0.5f * actor->shadowSize;
			shadowVertices[4 * shadowCount + 3].y = -0.49f;
			shadowVertices[4 * shadowCount + 3].z = actor->position.y + 0.5f * actor->shadowSize;

			shadowIndices[6 * shadowCount] = shadowCount * 4;
			shadowIndices[6 * shadowCount + 1] = shadowCount * 4 + 1;
			shadowIndices[6 * shadowCount + 2] = shadowCount * 4 + 2;
			shadowIndices[6 * shadowCount + 3] = shadowCount * 4;
			shadowIndices[6 * shadowCount + 4] = shadowCount * 4 + 2;
			shadowIndices[6 * shadowCount + 5] = shadowCount * 4 + 3;

			shadowCount++;
		}
	}
	ListUnlock(level->actors);
	lunaWriteDataToBuffer(buffers.wallActors.instanceData.buffer,
						  buffers.wallActors.instanceData.data,
						  buffers.wallActors.instanceData.bytesUsed);
	lunaWriteDataToBuffer(buffers.modelActors.instanceData.buffer,
						  buffers.modelActors.instanceData.data,
						  buffers.modelActors.instanceData.bytesUsed);
	lunaWriteDataToBuffer(buffers.shadows.vertices.buffer,
						  buffers.shadows.vertices.data,
						  buffers.shadows.vertices.bytesUsed);
	lunaWriteDataToBuffer(buffers.shadows.indices.buffer,
						  buffers.shadows.indices.data,
						  buffers.shadows.indices.bytesUsed);

	return VK_SUCCESS;
}
