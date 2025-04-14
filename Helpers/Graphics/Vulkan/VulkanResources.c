//
// Created by Noah on 12/18/2024.
//

#include "VulkanResources.h"
#include <luna/luna.h>
#include "../../../Structs/GlobalState.h"
#include "../../CommonAssets.h"
#include "../../Core/Error.h"
#include "../../Core/MathEx.h"

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
		void *newVertices = realloc(buffers.wallActors.vertices.data, buffers.wallActors.vertices.allocatedSize);
		CheckAlloc(newVertices);
		buffers.wallActors.vertices.data = newVertices;
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
		void *newIndices = realloc(buffers.wallActors.indices.data, buffers.wallActors.indices.allocatedSize);
		CheckAlloc(newIndices);
		buffers.wallActors.indices.data = newIndices;
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
		void *newInstanceData = realloc(buffers.wallActors.instanceData.data,
										buffers.wallActors.instanceData.allocatedSize);
		CheckAlloc(newInstanceData);
		memset(newInstanceData, 0, buffers.wallActors.instanceData.allocatedSize);
		buffers.wallActors.instanceData.data = newInstanceData;
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
		void *newDrawInfo = realloc(buffers.wallActors.drawInfo.data, buffers.wallActors.drawInfo.allocatedSize);
		CheckAlloc(newDrawInfo);
		memset(newDrawInfo, 0, buffers.wallActors.drawInfo.allocatedSize);
		buffers.wallActors.drawInfo.data = newDrawInfo;
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
		void *newVertices = realloc(buffers.modelActors.vertices.data, buffers.modelActors.vertices.allocatedSize);
		CheckAlloc(newVertices);
		buffers.modelActors.vertices.data = newVertices;
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
		void *newIndices = realloc(buffers.modelActors.indices.data, buffers.modelActors.indices.allocatedSize);
		CheckAlloc(newIndices);
		buffers.modelActors.indices.data = newIndices;
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
		void *newInstanceData = realloc(buffers.modelActors.instanceData.data,
										buffers.modelActors.instanceData.allocatedSize);
		CheckAlloc(newInstanceData);
		memset(newInstanceData, 0, buffers.modelActors.instanceData.allocatedSize);
		buffers.modelActors.instanceData.data = newInstanceData;
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
		void *newDrawInfo = realloc(buffers.modelActors.drawInfo.data, buffers.modelActors.drawInfo.allocatedSize);
		CheckAlloc(newDrawInfo);
		memset(newDrawInfo, 0, buffers.modelActors.drawInfo.allocatedSize);
		buffers.modelActors.drawInfo.data = newDrawInfo;
	}

	return VK_SUCCESS;
}

bool LoadTexture(const Image *image)
{
	const LunaSampledImageCreationInfo imageCreationInfo = {
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.width = image->width,
		.height = image->height,
		.mipmapLevels = GetState()->options.mipmaps && false ? (uint8_t)log2(max(image->width, image->height)) + 1 : 1,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.pixels = image->pixelData,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.sampler = textureSamplers.nearestRepeat,
		.sourceStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
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

	/*
	vkGetImageMemoryRequirements(device, texture->image, &texture->allocationInfo.memoryRequirements);
	if (textures.length == 1)
	{
		texture->allocationInfo.offset = 0;
	} else
	{
		const Texture *previousTexture = (Texture *)ListGet(textures, textures.length - 2);
		const VkDeviceSize alignment = texture->allocationInfo.memoryRequirements.alignment;
		const double previousSizeAligned = (double)(previousTexture->allocationInfo.offset +
													previousTexture->allocationInfo.memoryRequirements.size);

		texture->allocationInfo.offset = alignment * (VkDeviceSize)ceil(previousSizeAligned / (double)alignment);
	}
	imageAssetIdToIndexMap[image->id] = textures.length - 1;

	MemoryInfo stagingBufferMemoryInfo = {
		.type = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	};
	const MemoryAllocationInfo stagingBufferMemoryAllocationInfo = {
		.memoryInfo = &stagingBufferMemoryInfo,
		.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	};
	Buffer stagingBuffer = {
		.size = image->pixelDataSize,
		.memoryAllocationInfo = stagingBufferMemoryAllocationInfo,
	};
	if (!CreateBuffer(&stagingBuffer, true))
	{
		return false;
	}
	VulkanTest(vkBindImageMemory(device, texture->image, textureMemory.memory, texture->allocationInfo.offset),
			   "Failed to bind Vulkan texture memory!");

	void *data;
	VulkanTest(vkMapMemory(device, stagingBufferMemoryInfo.memory, 0, VK_WHOLE_SIZE, 0, &data),
			   "Failed to map Vulkan texture staging buffer memory!");

	memcpy(data, image->pixelData, image->pixelDataSize);
	vkUnmapMemory(device, stagingBufferMemoryInfo.memory);

	const VkCommandBuffer commandBuffer;
	if (!BeginCommandBuffer(&commandBuffer, graphicsCommandPool))
	{
		return false;
	}

	const VkImageSubresourceRange transferSubresourceRange = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.levelCount = texture->mipmapLevels,
		.layerCount = 1,
	};
	const VkImageMemoryBarrier transferBarrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = texture->image,
		.subresourceRange = transferSubresourceRange,
	};

	vkCmdPipelineBarrier(commandBuffer,
						 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						 VK_PIPELINE_STAGE_TRANSFER_BIT,
						 0,
						 0,
						 NULL,
						 0,
						 NULL,
						 1,
						 &transferBarrier);

	if (!EndCommandBuffer(commandBuffer, graphicsCommandPool, graphicsQueue))
	{
		return false;
	}

	if (!BeginCommandBuffer(&commandBuffer, graphicsCommandPool))
	{
		return false;
	}

	const VkImageSubresourceLayers subresourceLayers = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.layerCount = 1,
	};
	const VkExtent3D imageExtent = {
		.width = image->width,
		.height = image->height,
		.depth = 1,
	};
	const VkBufferImageCopy bufferCopyInfo = {
		.bufferOffset = 0,
		.imageSubresource = subresourceLayers,
		.imageExtent = imageExtent,
	};

	vkCmdCopyBufferToImage(commandBuffer,
						   stagingBuffer.buffer,
						   texture->image,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						   1,
						   &bufferCopyInfo);

	if (!EndCommandBuffer(commandBuffer, graphicsCommandPool, graphicsQueue))
	{
		return false;
	}


	if (!BeginCommandBuffer(&commandBuffer, graphicsCommandPool))
	{
		return false;
	}

	uint32_t width = image->width;
	uint32_t height = image->height;
	for (uint8_t level = 0; level < texture->mipmapLevels - 1; level++)
	{
		const VkImageSubresourceRange blitSubresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = level,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};
		const VkImageMemoryBarrier blitBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = NULL,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = texture->image,
			.subresourceRange = blitSubresourceRange,
		};

		vkCmdPipelineBarrier(commandBuffer,
							 VK_PIPELINE_STAGE_TRANSFER_BIT,
							 VK_PIPELINE_STAGE_TRANSFER_BIT,
							 0,
							 0,
							 NULL,
							 0,
							 NULL,
							 1,
							 &blitBarrier);

		VkImageBlit blit = {
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = level,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			{
				{0, 0, 0},
				{
					.x = (int32_t)width,
					.y = (int32_t)height,
					.z = 1,
				},
			},
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = level + 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			{
				{0, 0, 0},
				{
					.x = width > 1 ? (int32_t)width / 2 : 1,
					.y = height > 1 ? (int32_t)height / 2 : 1,
					.z = 1,
				},
			},
		};

		vkCmdBlitImage(commandBuffer,
					   texture->image,
					   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					   texture->image,
					   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					   1,
					   &blit,
					   VK_FILTER_LINEAR);

		const VkImageSubresourceRange mipmapSubresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = level,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};
		const VkImageMemoryBarrier mipmapBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = NULL,
			.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = texture->image,
			.subresourceRange = mipmapSubresourceRange,
		};

		// TODO Best practices validation doesn't like this
		vkCmdPipelineBarrier(commandBuffer,
							 VK_PIPELINE_STAGE_TRANSFER_BIT,
							 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
							 0,
							 0,
							 NULL,
							 0,
							 NULL,
							 1,
							 &mipmapBarrier);

		if (width > 1)
		{
			width /= 2;
		}
		if (height > 1)
		{
			height /= 2;
		}
	}

	const VkImageSubresourceRange mipmapSubresourceRange = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = texture->mipmapLevels - 1,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};
	const VkImageMemoryBarrier mipmapBarrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = texture->image,
		.subresourceRange = mipmapSubresourceRange,
	};

	vkCmdPipelineBarrier(commandBuffer,
						 VK_PIPELINE_STAGE_TRANSFER_BIT,
						 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						 0,
						 0,
						 NULL,
						 0,
						 NULL,
						 1,
						 &mipmapBarrier);

	if (!EndCommandBuffer(commandBuffer, graphicsCommandPool, graphicsQueue))
	{
		return false;
	}

	VkImageView *textureImageView = malloc(sizeof(VkImageView *));
	CheckAlloc(textureImageView);
	ListAdd(&texturesImageView, textureImageView);
	if (!CreateImageView(textureImageView,
						 texture->image,
						 VK_FORMAT_R8G8B8A8_UNORM,
						 VK_IMAGE_ASPECT_COLOR_BIT,
						 texture->mipmapLevels,
						 "Failed to create Vulkan texture image view!"))
	{
		return false;
	}

	if (!DestroyBuffer(&stagingBuffer))
	{
		return false;
	}

	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorImageInfo imageInfo = {
			.sampler = textureSamplers.nearestRepeat,
			.imageView = *textureImageView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};

		const VkWriteDescriptorSet writeDescriptor = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = descriptorSets[i],
			.dstBinding = 0,
			.dstArrayElement = textures.length - 1,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &imageInfo,
			.pBufferInfo = NULL,
			.pTexelBufferView = NULL,
		};
		vkUpdateDescriptorSets(device, 1, &writeDescriptor, 0, NULL);
	}
*/
	return true;
}
