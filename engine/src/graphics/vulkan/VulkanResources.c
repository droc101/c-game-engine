//
// Created by Noah on 12/18/2024.
//

#include <engine/assets/ModelLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanResources.h>
#include <engine/helpers/MathEx.h>
#include <engine/helpers/Realloc.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Viewmodel.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/threads/LodThread.h>
#include <luna/luna.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaImage.h>
#include <luna/lunaTypes.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

VkResult CreateUiBuffers()
{
	const LunaBufferCreationInfo vertexBufferCreationInfo = {
		.size = buffers.ui.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&vertexBufferCreationInfo, &buffers.ui.vertices.buffer),
						   "Failed to create UI vertex buffer!");
	buffers.ui.vertices.data = malloc(buffers.ui.vertices.allocatedSize);
	CheckAlloc(buffers.ui.vertices.data);

	const LunaBufferCreationInfo indexBufferCreationInfo = {
		.size = buffers.ui.indices.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&indexBufferCreationInfo, &buffers.ui.indices.buffer),
						   "Failed to create UI index buffer!");
	buffers.ui.indices.data = malloc(buffers.ui.indices.allocatedSize);
	CheckAlloc(buffers.ui.indices.data);

	return VK_SUCCESS;
}

VkResult CreateViewModelBuffers()
{
	const Viewmodel *viewmodel = &GetState()->map->viewmodel;
	const ModelDefinition *model = viewmodel->model;
	const size_t vertexSize = sizeof(ModelVertex) * model->lods[0]->vertexCount;
	const size_t indexSize = sizeof(uint32_t) * model->lods[0]->totalIndexCount;

	const LunaBufferCreationInfo vertexBufferCreationInfo = {
		.size = vertexSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	const LunaBufferCreationInfo indexBufferCreationInfo = {
		.size = indexSize,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	};
	const LunaBufferCreationInfo instanceDataBufferCreationInfo = {
		.size = sizeof(ModelInstanceData) * model->materialsPerSkin,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	const LunaBufferCreationInfo drawInfoBufferCreationInfo = {
		.size = sizeof(VkDrawIndexedIndirectCommand) * model->materialsPerSkin,
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&vertexBufferCreationInfo, &buffers.viewModel.vertices),
						   "Failed to create view model vertex buffer!");
	VulkanTestReturnResult(lunaCreateBuffer(&indexBufferCreationInfo, &buffers.viewModel.indices),
						   "Failed to create view model index buffer!");
	VulkanTestReturnResult(lunaCreateBuffer(&instanceDataBufferCreationInfo, &buffers.viewModel.instanceDataBuffer),
						   "Failed to create view model instance data buffer!");
	VulkanTestReturnResult(lunaCreateBuffer(&drawInfoBufferCreationInfo, &buffers.viewModel.drawInfo),
						   "Failed to create view model draw info buffer!");

	uint32_t indexCount = 0;
	void *vertexData = malloc(vertexSize);
	CheckAlloc(vertexData);
	void *indexData = malloc(indexSize);
	CheckAlloc(indexData);
	buffers.viewModel.instanceDatas = calloc(model->materialsPerSkin, sizeof(ModelInstanceData));
	CheckAlloc(buffers.viewModel.instanceDatas);
	VkDrawIndexedIndirectCommand *drawInfos = calloc(model->materialsPerSkin, sizeof(VkDrawIndexedIndirectCommand));
	CheckAlloc(drawInfos);
	memcpy(vertexData, model->lods[0]->vertexData, vertexSize);
	for (uint32_t i = 0; i < model->materialsPerSkin; i++)
	{
		const Material *material = &model->materials[model->skins[viewmodel->modelSkin][i]];
		memcpy(indexData + sizeof(uint32_t) * indexCount,
			   model->lods[0]->indexData[i],
			   sizeof(uint32_t) * model->lods[0]->indexCount[i]);
		buffers.viewModel.instanceDatas[i].transform[0][0] = 1;
		buffers.viewModel.instanceDatas[i].transform[1][1] = 1;
		buffers.viewModel.instanceDatas[i].transform[2][2] = 1;
		buffers.viewModel.instanceDatas[i].transform[3][3] = 1;
		buffers.viewModel.instanceDatas[i].textureIndex = TextureIndex(material->texture);
		buffers.viewModel.instanceDatas[i].materialColor = material->color;
		drawInfos[i].indexCount = model->lods[0]->indexCount[i];
		drawInfos[i].instanceCount = 1;
		drawInfos[i].firstIndex = indexCount;
		drawInfos[i].firstInstance = i;
		indexCount += model->lods[0]->indexCount[i];
	}

	buffers.viewModel.drawCount = model->materialsPerSkin;
	lunaWriteDataToBuffer(buffers.viewModel.vertices, vertexData, vertexSize, 0);
	lunaWriteDataToBuffer(buffers.viewModel.indices, indexData, indexSize, 0);
	lunaWriteDataToBuffer(buffers.viewModel.instanceDataBuffer,
						  buffers.viewModel.instanceDatas,
						  sizeof(ModelInstanceData) * model->materialsPerSkin,
						  0);
	lunaWriteDataToBuffer(buffers.viewModel.drawInfo,
						  drawInfos,
						  sizeof(VkDrawIndexedIndirectCommand) * model->materialsPerSkin,
						  0);

	free(vertexData);
	free(indexData);
	free(drawInfos);

	return VK_SUCCESS;
}

VkResult CreateWallBuffers()
{
	const LunaBufferCreationInfo vertexBufferCreationInfo = {
		.size = buffers.walls.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&vertexBufferCreationInfo, &buffers.walls.vertices.buffer),
						   "Failed to create wall vertex buffer!");
	buffers.walls.vertices.data = malloc(buffers.walls.vertices.allocatedSize);
	CheckAlloc(buffers.walls.vertices.data);

	const LunaBufferCreationInfo indexBufferCreationInfo = {
		.size = buffers.walls.indices.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&indexBufferCreationInfo, &buffers.walls.indices.buffer),
						   "Failed to create wall index buffer!");
	buffers.walls.indices.data = malloc(buffers.walls.indices.allocatedSize);
	CheckAlloc(buffers.walls.indices.data);

	return VK_SUCCESS;
}

VkResult CreateActorWallBuffers()
{
	const LunaBufferCreationInfo actorWallsVertexBufferCreationInfo = {
		.size = buffers.actorWalls.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&actorWallsVertexBufferCreationInfo, &buffers.actorWalls.vertices.buffer),
						   "Failed to create wall actors vertex buffer!");
	buffers.actorWalls.vertices.data = malloc(buffers.actorWalls.vertices.allocatedSize);
	CheckAlloc(buffers.actorWalls.vertices.data);

	const LunaBufferCreationInfo actorWallsIndexBufferCreationInfo = {
		.size = buffers.actorWalls.indices.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&actorWallsIndexBufferCreationInfo, &buffers.actorWalls.indices.buffer),
						   "Failed to create wall actors index buffer!");
	buffers.actorWalls.indices.data = malloc(buffers.actorWalls.indices.allocatedSize);
	CheckAlloc(buffers.actorWalls.indices.data);

	const LunaBufferCreationInfo actorWallsInstanceDataBufferCreationInfo = {
		.size = buffers.actorWalls.instanceData.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&actorWallsInstanceDataBufferCreationInfo,
											&buffers.actorWalls.instanceData.buffer),
						   "Failed to create wall actors instance data buffer!");
	buffers.actorWalls.instanceData.data = calloc(1, buffers.actorWalls.instanceData.allocatedSize);
	CheckAlloc(buffers.actorWalls.instanceData.data);

	const LunaBufferCreationInfo actorWallsDrawInfoBufferCreationInfo = {
		.size = buffers.actorWalls.drawInfo.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&actorWallsDrawInfoBufferCreationInfo, &buffers.actorWalls.drawInfo.buffer),
						   "Failed to create wall actors draw info buffer!");
	buffers.actorWalls.drawInfo.data = calloc(1, buffers.actorWalls.drawInfo.allocatedSize);
	CheckAlloc(buffers.actorWalls.drawInfo.data);

	return VK_SUCCESS;
}

VkResult CreateActorModelBuffers()
{
	const LunaBufferCreationInfo actorModelsVertexBufferCreationInfo = {
		.size = buffers.actorModels.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&actorModelsVertexBufferCreationInfo, &buffers.actorModels.vertices.buffer),
						   "Failed to create model actors vertex buffer!");
	buffers.actorModels.vertices.data = malloc(buffers.actorModels.vertices.allocatedSize);
	CheckAlloc(buffers.actorModels.vertices.data);

	const LunaBufferCreationInfo actorModelsIndexBufferCreationInfo = {
		.size = buffers.actorModels.indices.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&actorModelsIndexBufferCreationInfo, &buffers.actorModels.indices.buffer),
						   "Failed to create model actors index buffer!");
	buffers.actorModels.indices.data = malloc(buffers.actorModels.indices.allocatedSize);
	CheckAlloc(buffers.actorModels.indices.data);

	const LunaBufferCreationInfo actorModelsInstanceDataBufferCreationInfo = {
		.size = buffers.actorModels.instanceData.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&actorModelsInstanceDataBufferCreationInfo,
											&buffers.actorModels.instanceData.buffer),
						   "Failed to create model actors instance data buffer!");
	buffers.actorModels.instanceData.data = calloc(1, buffers.actorModels.instanceData.allocatedSize);
	CheckAlloc(buffers.actorModels.instanceData.data);

	const LunaBufferCreationInfo actorModelsShadedDrawInfoBufferCreationInfo = {
		.size = buffers.actorModels.shadedDrawInfo.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&actorModelsShadedDrawInfoBufferCreationInfo,
											&buffers.actorModels.shadedDrawInfo.buffer),
						   "Failed to create model actors shaded draw info buffer!");
	buffers.actorModels.shadedDrawInfo.data = calloc(1, buffers.actorModels.shadedDrawInfo.allocatedSize);
	CheckAlloc(buffers.actorModels.shadedDrawInfo.data);

	const LunaBufferCreationInfo actorModelsUnshadedDrawInfoBufferCreationInfo = {
		.size = buffers.actorModels.unshadedDrawInfo.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&actorModelsUnshadedDrawInfoBufferCreationInfo,
											&buffers.actorModels.unshadedDrawInfo.buffer),
						   "Failed to create model actors unshaded draw info buffer!");
	buffers.actorModels.unshadedDrawInfo.data = calloc(1, buffers.actorModels.unshadedDrawInfo.allocatedSize);
	CheckAlloc(buffers.actorModels.unshadedDrawInfo.data);

	return VK_SUCCESS;
}

VkResult CreateDebugDrawBuffers()
{
#ifdef JPH_DEBUG_RENDERER
	const LunaBufferCreationInfo linesBufferCreationInfo = {
		.size = buffers.debugDrawLines.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&linesBufferCreationInfo, &buffers.debugDrawLines.vertices.buffer),
						   "Failed to create debug draw lines buffer!");
	buffers.debugDrawLines.vertices.data = malloc(buffers.debugDrawLines.vertices.allocatedSize);
	CheckAlloc(buffers.debugDrawLines.vertices.data);

	const LunaBufferCreationInfo trianglesBufferCreationInfo = {
		.size = buffers.debugDrawTriangles.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&trianglesBufferCreationInfo, &buffers.debugDrawTriangles.vertices.buffer),
						   "Failed to create debug draw triangles buffer!");
	buffers.debugDrawTriangles.vertices.data = malloc(buffers.debugDrawTriangles.vertices.allocatedSize);
	CheckAlloc(buffers.debugDrawTriangles.vertices.data);
#endif

	return VK_SUCCESS;
}

VkResult ResizeWallBuffers()
{
	if (buffers.walls.vertices.allocatedSize < buffers.walls.vertices.bytesUsed)
	{
		if (buffers.walls.vertices.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.walls.vertices.buffer);
		}
		buffers.walls.vertices.allocatedSize = buffers.walls.vertices.bytesUsed;
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.walls.vertices.allocatedSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.walls.vertices.buffer),
							   "Failed to recreate wall vertex buffer!");
		void *newVertices = realloc(buffers.walls.vertices.data, buffers.walls.vertices.allocatedSize);
		CheckAlloc(newVertices);
		buffers.walls.vertices.data = newVertices;
	}
	if (buffers.walls.indices.allocatedSize < buffers.walls.indices.bytesUsed)
	{
		if (buffers.walls.indices.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.walls.indices.buffer);
		}
		buffers.walls.indices.allocatedSize = buffers.walls.indices.bytesUsed;
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.walls.indices.allocatedSize,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.walls.indices.buffer),
							   "Failed to recreate wall index buffer!");
		void *newIndices = realloc(buffers.walls.indices.data, buffers.walls.indices.allocatedSize);
		CheckAlloc(newIndices);
		buffers.walls.indices.data = newIndices;
	}
	return VK_SUCCESS;
}

VkResult ResizeActorWallBuffers()
{
	if (buffers.actorWalls.vertices.allocatedSize < buffers.actorWalls.vertices.bytesUsed)
	{
		if (buffers.actorWalls.vertices.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.actorWalls.vertices.buffer);
		}
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.actorWalls.vertices.bytesUsed,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.actorWalls.vertices.buffer),
							   "Failed to recreate wall actors vertex buffer!");
		ActorWallVertex *newVertices = realloc(buffers.actorWalls.vertices.data, buffers.actorWalls.vertices.bytesUsed);
		CheckAlloc(newVertices);
		buffers.actorWalls.vertices.data = newVertices;
		buffers.actorWalls.vertices.allocatedSize = buffers.actorWalls.vertices.bytesUsed;
	}
	if (buffers.actorWalls.indices.allocatedSize < buffers.actorWalls.indices.bytesUsed)
	{
		if (buffers.actorWalls.indices.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.actorWalls.indices.buffer);
		}
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.actorWalls.indices.bytesUsed,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.actorWalls.indices.buffer),
							   "Failed to recreate wall actors index buffer!");
		uint32_t *newIndices = realloc(buffers.actorWalls.indices.data, buffers.actorWalls.indices.bytesUsed);
		CheckAlloc(newIndices);
		buffers.actorWalls.indices.data = newIndices;
		buffers.actorWalls.indices.allocatedSize = buffers.actorWalls.indices.bytesUsed;
	}
	if (buffers.actorWalls.instanceData.allocatedSize < buffers.actorWalls.instanceData.bytesUsed)
	{
		if (buffers.actorWalls.instanceData.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.actorWalls.instanceData.buffer);
		}
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.actorWalls.instanceData.bytesUsed,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.actorWalls.instanceData.buffer),
							   "Failed to recreate wall actors instance data buffer!");
		buffers.actorWalls.instanceData.data = Recalloc(buffers.actorWalls.instanceData.data,
														buffers.actorWalls.instanceData.allocatedSize,
														buffers.actorWalls.instanceData.bytesUsed);
		CheckAlloc(buffers.actorWalls.instanceData.data);
		buffers.actorWalls.instanceData.allocatedSize = buffers.actorWalls.instanceData.bytesUsed;
	}
	if (buffers.actorWalls.drawInfo.allocatedSize < buffers.actorWalls.drawInfo.bytesUsed)
	{
		if (buffers.actorWalls.drawInfo.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.actorWalls.drawInfo.buffer);
		}
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.actorWalls.drawInfo.bytesUsed,
			.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.actorWalls.drawInfo.buffer),
							   "Failed to recreate wall actors draw info buffer!");
		buffers.actorWalls.drawInfo.data = Recalloc(buffers.actorWalls.drawInfo.data,
													buffers.actorWalls.drawInfo.allocatedSize,
													buffers.actorWalls.drawInfo.bytesUsed);
		CheckAlloc(buffers.actorWalls.drawInfo.data);
		buffers.actorWalls.drawInfo.allocatedSize = buffers.actorWalls.drawInfo.bytesUsed;
	}

	return VK_SUCCESS;
}

VkResult ResizeActorModelInstanceDataBuffer()
{
	if (buffers.actorModels.instanceData.allocatedSize < buffers.actorModels.instanceData.bytesUsed)
	{
		if (buffers.actorModels.instanceData.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.actorModels.instanceData.buffer);
		}
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.actorModels.instanceData.bytesUsed,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.actorModels.instanceData.buffer),
							   "Failed to recreate model actors instance data buffer!");
		buffers.actorModels.instanceData.data = Recalloc(buffers.actorModels.instanceData.data,
														 buffers.actorModels.instanceData.allocatedSize,
														 buffers.actorModels.instanceData.bytesUsed);
		CheckAlloc(buffers.actorModels.instanceData.data);
		buffers.actorModels.instanceData.allocatedSize = buffers.actorModels.instanceData.bytesUsed;
	}

	return VK_SUCCESS;
}

VkResult ResizeActorModelBuffers()
{
	if (buffers.actorModels.vertices.allocatedSize < buffers.actorModels.vertices.bytesUsed)
	{
		if (buffers.actorModels.vertices.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.actorModels.vertices.buffer);
		}
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.actorModels.vertices.bytesUsed,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.actorModels.vertices.buffer),
							   "Failed to recreate model actors shaded vertex buffer!");
		ModelVertex *newVertices = realloc(buffers.actorModels.vertices.data, buffers.actorModels.vertices.bytesUsed);
		CheckAlloc(newVertices);
		buffers.actorModels.vertices.data = newVertices;
		buffers.actorModels.vertices.allocatedSize = buffers.actorModels.vertices.bytesUsed;
	}
	if (buffers.actorModels.indices.allocatedSize < buffers.actorModels.indices.bytesUsed)
	{
		if (buffers.actorModels.indices.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.actorModels.indices.buffer);
		}
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.actorModels.indices.bytesUsed,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.actorModels.indices.buffer),
							   "Failed to recreate model actors index buffer!");
		uint32_t *newIndices = realloc(buffers.actorModels.indices.data, buffers.actorModels.indices.bytesUsed);
		CheckAlloc(newIndices);
		buffers.actorModels.indices.data = newIndices;
		buffers.actorModels.indices.allocatedSize = buffers.actorModels.indices.bytesUsed;
	}
	VulkanTestReturnResult(ResizeActorModelInstanceDataBuffer(),
						   "Failed to recreate model actors instance data buffer!");
	if (buffers.actorModels.shadedDrawInfo.allocatedSize < buffers.actorModels.shadedDrawInfo.bytesUsed)
	{
		if (buffers.actorModels.shadedDrawInfo.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.actorModels.shadedDrawInfo.buffer);
		}
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.actorModels.shadedDrawInfo.bytesUsed,
			.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.actorModels.shadedDrawInfo.buffer),
							   "Failed to recreate model actors shaded draw info buffer!");
		buffers.actorModels.shadedDrawInfo.data = Recalloc(buffers.actorModels.shadedDrawInfo.data,
														   buffers.actorModels.shadedDrawInfo.allocatedSize,
														   buffers.actorModels.shadedDrawInfo.bytesUsed);
		CheckAlloc(buffers.actorModels.shadedDrawInfo.data);
		buffers.actorModels.shadedDrawInfo.allocatedSize = buffers.actorModels.shadedDrawInfo.bytesUsed;
	}
	if (buffers.actorModels.unshadedDrawInfo.allocatedSize < buffers.actorModels.unshadedDrawInfo.bytesUsed)
	{
		if (buffers.actorModels.unshadedDrawInfo.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.actorModels.unshadedDrawInfo.buffer);
		}
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.actorModels.unshadedDrawInfo.bytesUsed,
			.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.actorModels.unshadedDrawInfo.buffer),
							   "Failed to recreate model actors unshaded draw info buffer!");
		buffers.actorModels.unshadedDrawInfo.data = Recalloc(buffers.actorModels.unshadedDrawInfo.data,
															 buffers.actorModels.unshadedDrawInfo.allocatedSize,
															 buffers.actorModels.unshadedDrawInfo.bytesUsed);
		CheckAlloc(buffers.actorModels.unshadedDrawInfo.data);
		buffers.actorModels.unshadedDrawInfo.allocatedSize = buffers.actorModels.unshadedDrawInfo.bytesUsed;
	}

	return VK_SUCCESS;
}

VkResult ResizeDebugDrawBuffers()
{
#ifdef JPH_DEBUG_RENDERER
	lunaDestroyBuffer(buffers.debugDrawLines.vertices.buffer);
	lunaDestroyBuffer(buffers.debugDrawTriangles.vertices.buffer);


	const LunaBufferCreationInfo linesBufferCreationInfo = {
		.size = buffers.debugDrawLines.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	const LunaBufferCreationInfo trianglesBufferCreationInfo = {
		.size = buffers.debugDrawTriangles.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&linesBufferCreationInfo, &buffers.debugDrawLines.vertices.buffer),
						   "Failed to recreate debug draw lines buffer!");
	VulkanTestReturnResult(lunaCreateBuffer(&trianglesBufferCreationInfo, &buffers.debugDrawTriangles.vertices.buffer),
						   "Failed to recreate debug draw triangles buffer!");

	buffers.debugDrawLines.shouldResize = false;
	buffers.debugDrawTriangles.shouldResize = false;
#endif

	return VK_SUCCESS;
}

bool LoadTexture(const Image *image)
{
	LockLodThreadMutex(); // TODO: This is not a great fix but it works ig
	const bool useMipmaps = GetState()->options.mipmaps && image->mipmaps;
	LunaSampler sampler = LUNA_NULL_HANDLE;
	if (image->filter && image->repeat)
	{
		sampler = textureSamplers.linearRepeat;
	}
	if (image->filter && !image->repeat)
	{
		sampler = textureSamplers.linearNoRepeat;
	}
	if (!image->filter && image->repeat)
	{
		sampler = textureSamplers.nearestRepeat;
	}
	if (!image->filter && !image->repeat)
	{
		sampler = textureSamplers.nearestNoRepeat;
	}
	const LunaSampledImageCreationInfo imageCreationInfo = {
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.width = image->width,
		.height = image->height,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.writeInfo.bytes = image->width * image->height * sizeof(uint32_t),
		.writeInfo.pixels = image->pixelData,
		.writeInfo.mipmapLevels = useMipmaps ? (uint8_t)log2(max(image->width, image->height)) + 1 : 1,
		.writeInfo.generateMipmaps = useMipmaps,
		.writeInfo.sourceStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		.writeInfo.destinationStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		.writeInfo.destinationAccessMask = VK_ACCESS_SHADER_READ_BIT,
		.sampler = sampler,
	};
	LunaImage lunaImage = VK_NULL_HANDLE;
	const size_t index = textures.length;
	VulkanTest(lunaCreateImage(&imageCreationInfo, &lunaImage), "Failed to create texture!");
	imageAssetIdToIndexMap[image->id] = index;
	ListInsertAfter(textures, index - 1, lunaImage);

	const LunaDescriptorImageInfo imageInfo = {
		.image = lunaImage,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};
	LunaWriteDescriptorSet writeDescriptors[MAX_FRAMES_IN_FLIGHT];
	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		writeDescriptors[i] = (LunaWriteDescriptorSet){
			.descriptorSet = descriptorSets[i],
			.bindingName = "Textures",
			.descriptorArrayElement = index,
			.descriptorCount = 1,
			.imageInfo = &imageInfo,
		};
	}
	lunaWriteDescriptorSets(MAX_FRAMES_IN_FLIGHT, writeDescriptors);
	UnlockLodThreadMutex();

	return true;
}
