//
// Created by NBT22 on 5/8/25.
//

#include "VulkanActors.h"
#include <assert.h>
#include <luna/luna.h>
#include "../RenderingHelpers.h"
#include "VulkanHelpers.h"
#include "VulkanResources.h"

static size_t loadedActorCount;
static List loadedLodSkins;
static List lodSkinOffsets;
static List lodSkinInstanceCounts;
static List instanceDataLodSkinCounts;

void VulkanActorsVariablesInit()
{
	ListInit(loadedLodSkins, LIST_UINT64);
	ListInit(lodSkinOffsets, LIST_UINT64);
	ListInit(lodSkinInstanceCounts, LIST_UINT32);
	ListInit(instanceDataLodSkinCounts, LIST_UINT32);
}

void VulkanActorsVariablesCleanup()
{
	ListFree(loadedLodSkins);
	ListFree(lodSkinOffsets);
	ListFree(lodSkinInstanceCounts);
	ListFree(instanceDataLodSkinCounts);
}

static inline void GetSizeOfLod(const ModelDefinition *model, const ModelLod *lod)
{
	buffers.actorModels.vertices.bytesUsed += sizeof(ModelVertex) * lod->vertexCount;
	buffers.actorModels.indices.bytesUsed += sizeof(uint32_t) * lod->totalIndexCount;
	for (uint32_t i = 0; i < model->skinCount; i++)
	{
		for (uint32_t j = 0; j < model->materialsPerSkin; j++)
		{
			switch (model->materials[model->skins[i][j]].shader)
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
	}
}

static inline VkResult PreSizeActorBuffers(const LockingList *actors)
{
	List loadedModels;
	ListInit(loadedModels, LIST_UINT32);
	for (size_t i = 0; i < loadedActorCount; i++)
	{
		assert(loadedActorCount == actors->length);
		const Actor *actor = ListGetPointer(*actors, i);
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
			if (ListFind(loadedModels, model->id) == SIZE_MAX)
			{
				for (uint32_t j = 0; j < model->lodCount; j++)
				{
					GetSizeOfLod(model, model->lods[j]);
				}
				ListAdd(loadedModels, model->id);
			}
			const uint64_t lodSkin = ((size_t)model->lods[actor->currentLod]->id << 32) | actor->currentSkinIndex;
			const size_t lodSkinIndex = ListFind(loadedLodSkins, lodSkin);
			if (lodSkinIndex == SIZE_MAX)
			{
				ListAdd(loadedLodSkins, lodSkin);
				ListAdd(lodSkinInstanceCounts, 1);
			} else
			{
				ListGetUint32(lodSkinInstanceCounts, lodSkinIndex)++;
			}
			buffers.actorModels.instanceData.bytesUsed += sizeof(ModelInstanceData) *
														  model->materialsPerSkin *
														  model->skinCount;
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
	ListFree(loadedModels);
	ListClear(loadedLodSkins);
	ListCopy(lodSkinInstanceCounts, instanceDataLodSkinCounts);

	return VK_SUCCESS;
}

static inline void LoadLodSkin(const ModelDefinition *model,
							   const uint32_t lodIndex,
							   const uint32_t skinIndex,
							   VkDrawIndexedIndirectCommand **shadedMaterialsDrawInfo,
							   VkDrawIndexedIndirectCommand **unshadedMaterialsDrawInfo,
							   List *materialDrawInfos,
							   List *lodSkinDrawInfoIndices,
							   List *loadedModelLods,
							   List *modelLodVertexOffsets,
							   List *modelLodIndexOffsets)
{
	const ModelLod *lod = model->lods[lodIndex];
	const size_t modelLodIndex = ListFind(*loadedModelLods, ((size_t)model->id << 32) | lodIndex);
	assert(modelLodIndex != SIZE_MAX);
	const uint64_t lodSkin = ((size_t)lod->id << 32) | skinIndex;
	const size_t lodSkinIndex = ListFind(loadedLodSkins, lodSkin);
	if (lodSkinIndex == SIZE_MAX)
	{
		if (lodSkinInstanceCounts.length > loadedLodSkins.length)
		{
			assert(lodSkinOffsets.length == loadedLodSkins.length + 1);
			const size_t instanceCount = ListGetUint32(lodSkinInstanceCounts, loadedLodSkins.length) *
										 model->materialsPerSkin;
			ListAdd(lodSkinOffsets, instanceCount + ListGetUint64(lodSkinOffsets, lodSkinOffsets.length - 1));
			buffers.actorModels.instanceData.bytesUsed += sizeof(ModelInstanceData) * instanceCount;
		}
		ListAdd(loadedLodSkins, lodSkin);
		ListAdd(*lodSkinDrawInfoIndices, materialDrawInfos->length);
		uint32_t firstIndex = ListGetUint32(*modelLodIndexOffsets, modelLodIndex);
		const int32_t vertexOffset = ListGetInt32(*modelLodVertexOffsets, modelLodIndex);
		for (uint32_t i = 0; i < model->materialsPerSkin; i++)
		{
			switch (model->materials[model->skins[skinIndex][i]].shader)
			{
				case SHADER_SHADED:
					ListAdd(*materialDrawInfos, *shadedMaterialsDrawInfo);
					(*shadedMaterialsDrawInfo)->indexCount += lod->indexCount[i];
					(*shadedMaterialsDrawInfo)->instanceCount++;
					(*shadedMaterialsDrawInfo)->firstIndex += firstIndex;
					(*shadedMaterialsDrawInfo)->vertexOffset += vertexOffset;
					(*shadedMaterialsDrawInfo)++;
					break;
				case SHADER_UNSHADED:
					ListAdd(*materialDrawInfos, *unshadedMaterialsDrawInfo);
					(*unshadedMaterialsDrawInfo)->indexCount += lod->indexCount[i];
					(*unshadedMaterialsDrawInfo)->instanceCount++;
					(*unshadedMaterialsDrawInfo)->firstIndex += firstIndex;
					(*unshadedMaterialsDrawInfo)->vertexOffset += vertexOffset;
					(*unshadedMaterialsDrawInfo)++;
					break;
				default:
					assert(false && "Invalid material shader!");
			}
			firstIndex += lod->indexCount[i];
		}
	} else
	{
		const size_t lodSkinMaterialIndex = ListGetUint64(*lodSkinDrawInfoIndices, lodSkinIndex);
		for (uint32_t i = 0; i < model->materialsPerSkin; i++)
		{
			VkDrawIndexedIndirectCommand *drawInfo = ListGetPointer(*materialDrawInfos, lodSkinMaterialIndex + i);
			drawInfo->instanceCount++;
		}
	}
}

static inline void LoadActorModel(const ModelDefinition *model,
								  List *loadedModelLods,
								  List *modelLodVertexOffsets,
								  List *modelLodIndexOffsets)
{
	for (uint32_t i = 0; i < model->lodCount; i++)
	{
		const ModelLod *lod = model->lods[i];
		const uint64_t modelLod = ((size_t)model->id << 32) | i;
		ListAdd(*loadedModelLods, modelLod);
		ListAdd(*modelLodVertexOffsets, (int32_t)(buffers.actorModels.vertices.bytesUsed / sizeof(ModelVertex)));
		ListAdd(*modelLodIndexOffsets, (uint32_t)(buffers.actorModels.indices.bytesUsed / sizeof(uint32_t)));
		const size_t verticesSize = sizeof(ModelVertex) * lod->vertexCount;
		memcpy(buffers.actorModels.vertices.data + buffers.actorModels.vertices.bytesUsed,
			   lod->vertexData,
			   verticesSize);
		buffers.actorModels.vertices.bytesUsed += verticesSize;
		for (size_t j = 0; j < model->materialsPerSkin; j++)
		{
			const size_t indicesSize = sizeof(uint32_t) * lod->indexCount[j];
			memcpy(buffers.actorModels.indices.data + buffers.actorModels.indices.bytesUsed,
				   lod->indexData[j],
				   indicesSize);
			buffers.actorModels.indices.bytesUsed += indicesSize;
		}
	}
}

static inline VkResult InitActors(const LockingList *actors)
{
	assert(actors);
	memset(buffers.actorModels.shadedDrawInfo.data, 0, buffers.actorModels.shadedDrawInfo.bytesUsed);
	memset(buffers.actorModels.unshadedDrawInfo.data, 0, buffers.actorModels.unshadedDrawInfo.bytesUsed);
	memset(buffers.actorWalls.drawInfo.data, 0, buffers.actorWalls.drawInfo.bytesUsed);
	buffers.actorWalls.count = 0;
	buffers.actorModels.vertices.bytesUsed = 0;
	buffers.actorModels.indices.bytesUsed = 0;
	buffers.actorModels.instanceData.bytesUsed = 0;
	buffers.actorModels.shadedDrawInfo.bytesUsed = 0;
	buffers.actorModels.unshadedDrawInfo.bytesUsed = 0;
	loadedActorCount = actors->length;
	ListClear(loadedLodSkins);
	ListClear(lodSkinOffsets);
	ListAdd(lodSkinOffsets, 0);
	ListClear(lodSkinInstanceCounts);
	ListClear(instanceDataLodSkinCounts);

	VulkanTestReturnResult(PreSizeActorBuffers(actors), "Failed to pre-size actor buffers!");

	VkDrawIndexedIndirectCommand *shadedActorModelsDrawInfo = buffers.actorModels.shadedDrawInfo.data;
	VkDrawIndexedIndirectCommand *unshadedActorModelsDrawInfo = buffers.actorModels.unshadedDrawInfo.data;
	VkDrawIndexedIndirectCommand *actorWallsDrawInfo = buffers.actorWalls.drawInfo.data;
	List loadedModels;
	List materialDrawInfos;
	List lodSkinDrawInfoIndices;
	List loadedModelLods;
	List modelLodVertexOffsets;
	List modelLodIndexOffsets;
	ListInit(loadedModels, LIST_UINT32);
	ListInit(materialDrawInfos, LIST_POINTER);
	ListInit(lodSkinDrawInfoIndices, LIST_UINT64);
	ListInit(loadedModelLods, LIST_UINT64);
	ListInit(modelLodVertexOffsets, LIST_INT32);
	ListInit(modelLodIndexOffsets, LIST_UINT32);
	for (size_t i = 0; i < loadedActorCount; i++)
	{
		assert(loadedActorCount == actors->length);
		const Actor *actor = ListGetPointer(*actors, i);
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
			const ModelDefinition *model = actor->actorModel;
			if (ListFind(loadedModels, model->id) == SIZE_MAX)
			{
				LoadActorModel(model, &loadedModelLods, &modelLodVertexOffsets, &modelLodIndexOffsets);
				LoadLodSkin(model,
							actor->currentLod,
							actor->currentSkinIndex,
							&shadedActorModelsDrawInfo,
							&unshadedActorModelsDrawInfo,
							&materialDrawInfos,
							&lodSkinDrawInfoIndices,
							&loadedModelLods,
							&modelLodVertexOffsets,
							&modelLodIndexOffsets);
				ListAdd(loadedModels, model->id);
			} else
			{
				LoadLodSkin(model,
							actor->currentLod,
							actor->currentSkinIndex,
							&shadedActorModelsDrawInfo,
							&unshadedActorModelsDrawInfo,
							&materialDrawInfos,
							&lodSkinDrawInfoIndices,
							&loadedModelLods,
							&modelLodVertexOffsets,
							&modelLodIndexOffsets);
			}
		}
	}
	buffers.actorModels.shadedDrawInfo.bytesUsed = (void *)shadedActorModelsDrawInfo -
												   buffers.actorModels.shadedDrawInfo.data;
	buffers.actorModels.unshadedDrawInfo.bytesUsed = (void *)unshadedActorModelsDrawInfo -
													 buffers.actorModels.unshadedDrawInfo.data;
	VulkanTestReturnResult(ResizeActorModelInstanceDataBuffer(),
						   "Failed to recreate model actors instance data buffer!");
	assert(buffers.actorWalls.vertices.bytesUsed <= sizeof(ActorWallVertex) * 4 * buffers.actorWalls.count);
	assert(buffers.actorWalls.indices.bytesUsed <= sizeof(uint32_t) * 6 * buffers.actorWalls.count);
	assert(buffers.actorWalls.instanceData.bytesUsed <= sizeof(ActorWallInstanceData) * buffers.actorWalls.count);
	assert(buffers.actorWalls.drawInfo.bytesUsed <= sizeof(VkDrawIndexedIndirectCommand) * buffers.actorWalls.count);
	assert(buffers.actorModels.vertices.bytesUsed <= buffers.actorModels.vertices.allocatedSize);
	assert(buffers.actorModels.indices.bytesUsed <= buffers.actorModels.indices.allocatedSize);
	assert(buffers.actorModels.instanceData.bytesUsed <= buffers.actorModels.instanceData.allocatedSize);
	assert(buffers.actorModels.shadedDrawInfo.bytesUsed <= buffers.actorModels.shadedDrawInfo.allocatedSize);
	assert(buffers.actorModels.unshadedDrawInfo.bytesUsed <= buffers.actorModels.unshadedDrawInfo.allocatedSize);

	size_t totalInstanceCount = 0;
	for (size_t i = 0; i < materialDrawInfos.length; i++)
	{
		VkDrawIndexedIndirectCommand *drawInfo = ListGetPointer(materialDrawInfos, i);
		drawInfo->firstInstance = totalInstanceCount;
		totalInstanceCount += drawInfo->instanceCount;
	}

	ListFree(loadedModels);
	ListFree(materialDrawInfos);
	ListFree(lodSkinDrawInfoIndices);
	ListFree(loadedModelLods);
	ListFree(modelLodVertexOffsets);
	ListFree(modelLodIndexOffsets);

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

// static inline VkResult ReloadActors() {}

static inline VkResult LoadActorWalls(const LockingList *actors)
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
		const Actor *actor = ListGetPointer(*actors, i);
		if (!actor->actorWall || actor->actorModel != NULL)
		{
			continue;
		}
		const ActorWall *wall = actor->actorWall;
		const float halfHeight = wall->height / 2.0f;
		const vec2 startVertex = {wall->a.x, wall->a.y};
		const vec2 endVertex = {wall->b.x, wall->b.y};
		const vec2 startUV = {wall->uvOffset, 0};
		const vec2 endUV = {wall->uvScale * wall->length + wall->uvOffset, 1};

		vertices[4 * wallCount].position.x = startVertex[0];
		vertices[4 * wallCount].position.y = halfHeight;
		vertices[4 * wallCount].position.z = startVertex[1];
		vertices[4 * wallCount].u = startUV[0];
		vertices[4 * wallCount].v = startUV[1];

		vertices[4 * wallCount + 1].position.x = endVertex[0];
		vertices[4 * wallCount + 1].position.y = halfHeight;
		vertices[4 * wallCount + 1].position.z = endVertex[1];
		vertices[4 * wallCount + 1].u = endUV[0];
		vertices[4 * wallCount + 1].v = startUV[1];

		vertices[4 * wallCount + 2].position.x = endVertex[0];
		vertices[4 * wallCount + 2].position.y = -halfHeight;
		vertices[4 * wallCount + 2].position.z = endVertex[1];
		vertices[4 * wallCount + 2].u = endUV[0];
		vertices[4 * wallCount + 2].v = endUV[1];

		vertices[4 * wallCount + 3].position.x = startVertex[0];
		vertices[4 * wallCount + 3].position.y = -halfHeight;
		vertices[4 * wallCount + 3].position.z = startVertex[1];
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

static inline VkResult UpdateActorInstanceData(const LockingList *actors)
{
	assert(actors);
	ListZero(instanceDataLodSkinCounts);
	uint32_t wallCount = 0;
	if (__builtin_expect(loadedActorCount != actors->length, false))
	{
		VulkanTestReturnResult(InitActors(actors), "Failed to init actors!");
	}
	ActorWallInstanceData *actorWallsInstanceData = buffers.actorWalls.instanceData.data;
	for (size_t i = 0; i < loadedActorCount; i++)
	{
		const Actor *actor = ListGetPointer(*actors, i);
		if (!actor->actorWall && !actor->actorModel)
		{
			continue;
		}

		mat4 transformMatrix = GLM_MAT4_IDENTITY_INIT;
		ActorTransformMatrix(actor, &transformMatrix);
		if (actor->actorModel)
		{
			const ModelLod *lod = actor->actorModel->lods[actor->currentLod];
			const uint64_t lodSkin = ((size_t)lod->id << 32) | actor->currentSkinIndex;
			const size_t lodSkinIndex = ListFind(loadedLodSkins, lodSkin);
			if (lodSkinIndex == SIZE_MAX)
			{
				VulkanTestReturnResult(InitActors(actors), "Failed to init actors!");
				return VK_SUCCESS;
			}
			const uint64_t offset = sizeof(ModelInstanceData) *
									(ListGetUint64(lodSkinOffsets, lodSkinIndex) +
									 ListGetUint32(instanceDataLodSkinCounts, lodSkinIndex));
			assert(offset < buffers.actorModels.instanceData.allocatedSize);
			ModelInstanceData *offsetInstanceData = buffers.actorModels.instanceData.data + offset;
			const size_t instanceCount = ListGetUint32(lodSkinInstanceCounts, lodSkinIndex);
			for (size_t j = 0; j < actor->actorModel->materialsPerSkin; j++)
			{
				const size_t index = j * instanceCount;
				const size_t materialIndex = actor->actorModel->skins[actor->currentSkinIndex][j];
				const Material *material = &actor->actorModel->materials[materialIndex];
				memcpy(offsetInstanceData[index].transform, transformMatrix, sizeof(mat4));
				offsetInstanceData[index].textureIndex = TextureIndex(material->texture);
				offsetInstanceData[index].materialColor = material->color;
				offsetInstanceData[index].instanceColor = actor->modColor;
			}
			ListGetUint32(instanceDataLodSkinCounts, lodSkinIndex)++;
		} else if (actor->actorWall)
		{
			const ActorWall *wall = actor->actorWall;
			memcpy(actorWallsInstanceData[wallCount].transform, transformMatrix, sizeof(mat4));
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

bool VK_UpdateActors(const LockingList *actors, const bool shouldReloadActors)
{
	if (shouldReloadActors)
	{
		VulkanTest(InitActors(actors), "Failed to reload actors");
	}
	VulkanTest(LoadActorWalls(actors), "Failed to load wall actors!");
	VulkanTest(UpdateActorInstanceData(actors), "Failed to update actor instance data!");
	return true;
}
