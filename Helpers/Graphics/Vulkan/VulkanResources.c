//
// Created by Noah on 12/18/2024.
//

#include "VulkanResources.h"
#include <luna/luna.h>
#include "../../../Structs/GlobalState.h"
#include "../../Core/Error.h"
#include "../../Core/MathEx.h"
#include "VulkanHelpers.h"

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

VkResult CreateShadowBuffers()
{
	const LunaBufferCreationInfo vertexBufferCreationInfo = {
		.size = buffers.shadows.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&vertexBufferCreationInfo, &buffers.shadows.vertices.buffer),
						   "Failed to create shadow vertex buffer!");
	buffers.shadows.vertices.data = malloc(buffers.shadows.vertices.allocatedSize);
	CheckAlloc(buffers.shadows.vertices.data);

	const LunaBufferCreationInfo indexBufferCreationInfo = {
		.size = buffers.shadows.indices.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&indexBufferCreationInfo, &buffers.shadows.indices.buffer),
						   "Failed to create shadow index buffer!");
	buffers.shadows.indices.data = malloc(buffers.shadows.indices.allocatedSize);
	CheckAlloc(buffers.shadows.indices.data);

	return VK_SUCCESS;
}

VkResult CreateWallActorBuffers()
{
	const LunaBufferCreationInfo wallActorsVertexBufferCreationInfo = {
		.size = buffers.wallActors.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&wallActorsVertexBufferCreationInfo, &buffers.wallActors.vertices.buffer),
						   "Failed to create wall actors vertex buffer!");
	buffers.wallActors.vertices.data = malloc(buffers.wallActors.vertices.allocatedSize);
	CheckAlloc(buffers.wallActors.vertices.data);

	const LunaBufferCreationInfo wallActorsIndexBufferCreationInfo = {
		.size = buffers.wallActors.indices.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&wallActorsIndexBufferCreationInfo, &buffers.wallActors.indices.buffer),
						   "Failed to create wall actors index buffer!");
	buffers.wallActors.indices.data = malloc(buffers.wallActors.indices.allocatedSize);
	CheckAlloc(buffers.wallActors.indices.data);

	const LunaBufferCreationInfo wallActorsInstanceDataBufferCreationInfo = {
		.size = buffers.wallActors.instanceData.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&wallActorsInstanceDataBufferCreationInfo,
											&buffers.wallActors.instanceData.buffer),
						   "Failed to create wall actors instance data buffer!");
	buffers.wallActors.instanceData.data = calloc(1, buffers.wallActors.instanceData.allocatedSize);
	CheckAlloc(buffers.wallActors.instanceData.data);

	const LunaBufferCreationInfo wallActorsDrawInfoBufferCreationInfo = {
		.size = buffers.wallActors.drawInfo.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&wallActorsDrawInfoBufferCreationInfo, &buffers.wallActors.drawInfo.buffer),
						   "Failed to create wall actors draw info buffer!");
	buffers.wallActors.drawInfo.data = calloc(1, buffers.wallActors.drawInfo.allocatedSize);
	CheckAlloc(buffers.wallActors.drawInfo.data);

	return VK_SUCCESS;
}

VkResult CreateModelActorBuffers()
{
	const LunaBufferCreationInfo modelActorsVertexBufferCreationInfo = {
		.size = buffers.modelActors.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&modelActorsVertexBufferCreationInfo, &buffers.modelActors.vertices.buffer),
						   "Failed to create model actors vertex buffer!");
	buffers.modelActors.vertices.data = malloc(buffers.modelActors.vertices.allocatedSize);
	CheckAlloc(buffers.modelActors.vertices.data);

	const LunaBufferCreationInfo modelActorsIndexBufferCreationInfo = {
		.size = buffers.modelActors.indices.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&modelActorsIndexBufferCreationInfo, &buffers.modelActors.indices.buffer),
						   "Failed to create model actors index buffer!");
	buffers.modelActors.indices.data = malloc(buffers.modelActors.indices.allocatedSize);
	CheckAlloc(buffers.modelActors.indices.data);

	const LunaBufferCreationInfo modelActorsInstanceDataBufferCreationInfo = {
		.size = buffers.modelActors.instanceData.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&modelActorsInstanceDataBufferCreationInfo,
											&buffers.modelActors.instanceData.buffer),
						   "Failed to create model actors instance data buffer!");
	buffers.modelActors.instanceData.data = calloc(1, buffers.modelActors.instanceData.allocatedSize);
	CheckAlloc(buffers.modelActors.instanceData.data);

	const LunaBufferCreationInfo modelActorsDrawInfoBufferCreationInfo = {
		.size = buffers.modelActors.drawInfo.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&modelActorsDrawInfoBufferCreationInfo,
											&buffers.modelActors.drawInfo.buffer),
						   "Failed to create model actors draw info buffer!");
	buffers.modelActors.drawInfo.data = calloc(1, buffers.modelActors.drawInfo.allocatedSize);
	CheckAlloc(buffers.modelActors.drawInfo.data);

	return VK_SUCCESS;
}

VkResult CreateRoofBuffers()
{
	const Model *skyModel = LoadModel(MODEL("model_sky"));
	buffers.roof.vertices.allocatedSize = sizeof(WallVertex) * skyModel->vertexCount;
	buffers.roof.indices.allocatedSize = sizeof(uint32_t) * skyModel->indexCount;

	const LunaBufferCreationInfo vertexBufferCreationInfo = {
		.size = buffers.roof.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&vertexBufferCreationInfo, &buffers.roof.vertices.buffer),
						   "Failed to create roof vertex buffer!");
	buffers.roof.vertices.data = malloc(buffers.roof.vertices.allocatedSize);
	CheckAlloc(buffers.roof.vertices.data);

	const LunaBufferCreationInfo indexBufferCreationInfo = {
		.size = buffers.roof.indices.allocatedSize,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&indexBufferCreationInfo, &buffers.roof.indices.buffer),
						   "Failed to create roof index buffer!");
	buffers.roof.indices.data = malloc(buffers.roof.indices.allocatedSize);
	CheckAlloc(buffers.roof.indices.data);

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

VkResult ResizeShadowBuffers()
{
	if (buffers.shadows.vertices.allocatedSize < buffers.shadows.vertices.bytesUsed)
	{
		if (buffers.shadows.vertices.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.shadows.vertices.buffer);
		}
		buffers.shadows.vertices.allocatedSize = buffers.shadows.vertices.bytesUsed;
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.shadows.vertices.allocatedSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.shadows.vertices.buffer),
							   "Failed to recreate shadow vertex buffer!");
		void *newVertices = realloc(buffers.shadows.vertices.data, buffers.shadows.vertices.allocatedSize);
		CheckAlloc(newVertices);
		buffers.shadows.vertices.data = newVertices;
	}
	if (buffers.shadows.indices.allocatedSize < buffers.shadows.indices.bytesUsed)
	{
		if (buffers.shadows.indices.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.shadows.indices.buffer);
		}
		buffers.shadows.indices.allocatedSize = buffers.shadows.indices.bytesUsed;
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.shadows.indices.allocatedSize,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.shadows.indices.buffer),
							   "Failed to recreate shadow index buffer!");
		void *newIndices = realloc(buffers.shadows.indices.data, buffers.shadows.indices.allocatedSize);
		CheckAlloc(newIndices);
		buffers.shadows.indices.data = newIndices;
	}
	return VK_SUCCESS;
}

VkResult ResizeWallActorBuffers()
{
	if (buffers.wallActors.vertices.allocatedSize < buffers.wallActors.vertices.bytesUsed)
	{
		if (buffers.wallActors.vertices.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.wallActors.vertices.buffer);
		}
		buffers.wallActors.vertices.allocatedSize = buffers.wallActors.vertices.bytesUsed;
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.wallActors.vertices.allocatedSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.wallActors.vertices.buffer),
							   "Failed to recreate wall actors vertex buffer!");
		buffers.wallActors.vertices.data = malloc(buffers.wallActors.vertices.allocatedSize);
		CheckAlloc(buffers.wallActors.vertices.data);
	}
	if (buffers.wallActors.indices.allocatedSize < buffers.wallActors.indices.bytesUsed)
	{
		if (buffers.wallActors.indices.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.wallActors.indices.buffer);
		}
		buffers.wallActors.indices.allocatedSize = buffers.wallActors.indices.bytesUsed;
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.wallActors.indices.allocatedSize,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.wallActors.indices.buffer),
							   "Failed to recreate wall actors index buffer!");
		buffers.wallActors.indices.data = malloc(buffers.wallActors.indices.allocatedSize);
		CheckAlloc(buffers.wallActors.indices.data);
	}
	if (buffers.wallActors.instanceData.allocatedSize < buffers.wallActors.instanceData.bytesUsed)
	{
		if (buffers.wallActors.instanceData.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.wallActors.instanceData.buffer);
		}
		buffers.wallActors.instanceData.allocatedSize = buffers.wallActors.instanceData.bytesUsed;
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.wallActors.instanceData.allocatedSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.wallActors.instanceData.buffer),
							   "Failed to recreate wall actors instance data buffer!");
		buffers.wallActors.instanceData.data = calloc(1, buffers.wallActors.instanceData.allocatedSize);
		CheckAlloc(buffers.wallActors.instanceData.data);
	}
	if (buffers.wallActors.drawInfo.allocatedSize < buffers.wallActors.drawInfo.bytesUsed)
	{
		if (buffers.wallActors.drawInfo.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.wallActors.drawInfo.buffer);
		}
		buffers.wallActors.drawInfo.allocatedSize = buffers.wallActors.drawInfo.bytesUsed;
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.wallActors.drawInfo.allocatedSize,
			.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.wallActors.drawInfo.buffer),
							   "Failed to recreate wall actors draw info buffer!");
		buffers.wallActors.drawInfo.data = calloc(1, buffers.wallActors.drawInfo.allocatedSize);
		CheckAlloc(buffers.wallActors.drawInfo.data);
	}

	return VK_SUCCESS;
}

VkResult ResizeModelActorBuffers()
{
	if (buffers.modelActors.vertices.allocatedSize < buffers.modelActors.vertices.bytesUsed)
	{
		if (buffers.modelActors.vertices.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.modelActors.vertices.buffer);
		}
		buffers.modelActors.vertices.allocatedSize = buffers.modelActors.vertices.bytesUsed;
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.modelActors.vertices.allocatedSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.modelActors.vertices.buffer),
							   "Failed to recreate model actors vertex buffer!");
		buffers.modelActors.vertices.data = malloc(buffers.modelActors.vertices.allocatedSize);
		CheckAlloc(buffers.modelActors.vertices.data);
	}
	if (buffers.modelActors.indices.allocatedSize < buffers.modelActors.indices.bytesUsed)
	{
		if (buffers.modelActors.indices.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.modelActors.indices.buffer);
		}
		buffers.modelActors.indices.allocatedSize = buffers.modelActors.indices.bytesUsed;
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.modelActors.indices.allocatedSize,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.modelActors.indices.buffer),
							   "Failed to recreate model actors index buffer!");
		buffers.modelActors.indices.data = malloc(buffers.modelActors.indices.allocatedSize);
		CheckAlloc(buffers.modelActors.indices.data);
	}
	if (buffers.modelActors.instanceData.allocatedSize < buffers.modelActors.instanceData.bytesUsed)
	{
		if (buffers.modelActors.instanceData.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.modelActors.instanceData.buffer);
		}
		buffers.modelActors.instanceData.allocatedSize = buffers.modelActors.instanceData.bytesUsed;
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.modelActors.instanceData.allocatedSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.modelActors.instanceData.buffer),
							   "Failed to recreate model actors instance data buffer!");
		buffers.modelActors.instanceData.data = calloc(1, buffers.modelActors.instanceData.allocatedSize);
		CheckAlloc(buffers.modelActors.instanceData.data);
	}
	if (buffers.modelActors.drawInfo.allocatedSize < buffers.modelActors.drawInfo.bytesUsed)
	{
		if (buffers.modelActors.drawInfo.allocatedSize != 0)
		{
			lunaDestroyBuffer(buffers.modelActors.drawInfo.buffer);
		}
		buffers.modelActors.drawInfo.allocatedSize = buffers.modelActors.drawInfo.bytesUsed;
		const LunaBufferCreationInfo creationInfo = {
			.size = buffers.modelActors.drawInfo.allocatedSize,
			.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&creationInfo, &buffers.modelActors.drawInfo.buffer),
							   "Failed to recreate model actors draw info buffer!");
		buffers.modelActors.drawInfo.data = calloc(1, buffers.modelActors.drawInfo.allocatedSize);
		CheckAlloc(buffers.modelActors.drawInfo.data);
	}

	return VK_SUCCESS;
}

bool LoadTexture(const Image *image)
{
	const LunaSampledImageCreationInfo imageCreationInfo = {
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.width = image->width,
		.height = image->height,
		.mipmapLevels = GetState()->options.mipmaps ? (uint8_t)log2(max(image->width, image->height)) + 1 : 1,
		.generateMipmaps = GetState()->options.mipmaps,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.pixels = image->pixelData,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.sampler = textureSamplers.nearestRepeat,
		.sourceStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	};
	ListAdd(&textures, NULL);
	VulkanTest(lunaCreateImage(&imageCreationInfo, (LunaImage *)&textures.data[textures.length - 1]),
			   "Failed to create texture!");
	imageAssetIdToIndexMap[image->id] = textures.length - 1;

	const LunaDescriptorImageInfo imageInfo = {
		.image = textures.data[textures.length - 1],
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};
	LunaWriteDescriptorSet writeDescriptors[MAX_FRAMES_IN_FLIGHT];
	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		writeDescriptors[i] = (LunaWriteDescriptorSet){
			.descriptorSet = descriptorSets[i],
			.bindingName = "Textures",
			.descriptorArrayElement = textures.length - 1,
			.descriptorCount = 1,
			.imageInfo = &imageInfo,
		};
	}
	lunaWriteDescriptorSets(MAX_FRAMES_IN_FLIGHT, writeDescriptors);

	return true;
}
