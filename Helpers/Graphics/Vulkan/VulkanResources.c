//
// Created by Noah on 12/18/2024.
//

#include "VulkanResources.h"
#include "../../../Structs/GlobalState.h"
#include "../../CommonAssets.h"
#include "../../Core/Error.h"
#include "../../Core/MathEx.h"
#include "VulkanMemory.h"

bool CreateLocalBuffer()
{
	buffers.walls.shadowSize = sizeof(ShadowVertex) * buffers.walls.shadowCount * 4 +
							   sizeof(uint32_t) * buffers.walls.shadowCount * 6;

	buffers.ui.vertexSize = sizeof(UiVertex) * buffers.ui.maxQuads * 4;
	buffers.ui.indexSize = sizeof(uint32_t) * buffers.ui.maxQuads * 6;

	buffers.actors.instanceDataSize = sizeof(ActorInstanceData) * buffers.actors.walls.count;
	if (buffers.actors.models.modelCounts.length && buffers.actors.models.loadedModelIds.length)
	{
		for (size_t i = 0; i < buffers.actors.models.loadedModelIds.length; i++)
		{
			buffers.actors.instanceDataSize += sizeof(ActorInstanceData) *
											   (size_t)ListGet(buffers.actors.models.modelCounts, i);
		}
	}
	buffers.actors.drawInfoSize = sizeof(VkDrawIndexedIndirectCommand) * buffers.actors.models.loadedModelIds.length +
								  sizeof(VkDrawIndexedIndirectCommand) * buffers.actors.walls.count;

	buffers.actors.models.vertexSize = sizeof(ActorVertex) * buffers.actors.models.vertexCount;
	buffers.actors.models.indexSize = sizeof(uint32_t) * buffers.actors.models.indexCount;
	buffers.actors.walls.vertexSize = sizeof(ActorVertex) * buffers.actors.walls.count * 4;
	buffers.actors.walls.indexSize = sizeof(uint32_t) * buffers.actors.walls.count * 6;

	// const ModelDefinition *_skyModel = !skyModel ? LoadModel(MODEL("model_sky")) : skyModel;
	buffers.local.size = sizeof(WallVertex) * (buffers.walls.maxWallCount * 4 + 0 /*_skyModel->vertexCount*/) +
						 sizeof(uint32_t) * (buffers.walls.maxWallCount * 6 + 0 /*_skyModel->indexCount*/) +
						 buffers.walls.shadowSize +
						 buffers.ui.vertexSize +
						 buffers.ui.indexSize +
						 buffers.actors.instanceDataSize +
						 buffers.actors.drawInfoSize +
						 buffers.actors.models.vertexSize +
						 buffers.actors.models.indexSize +
						 buffers.actors.walls.vertexSize +
						 buffers.actors.walls.indexSize;


	buffers.local.memoryAllocationInfo.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
													VK_BUFFER_USAGE_TRANSFER_DST_BIT |
													VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
													VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
													VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

	buffers.local.memoryAllocationInfo.memoryInfo = &memoryPools.localMemory;
	if (!CreateBuffer(&buffers.local, false))
	{
		return false;
	}

	return true;
}

void SetLocalBufferAliasingInfo()
{
	// const ModelDefinition *_skyModel = !skyModel ? LoadModel(MODEL("model_sky")) : skyModel;
	const VkDeviceSize wallVertexSize = sizeof(WallVertex) *
										(buffers.walls.maxWallCount * 4 + 0 /*_skyModel->vertexCount*/);
	const VkDeviceSize wallIndexSize = sizeof(uint32_t) *
									   (buffers.walls.maxWallCount * 6 + 0 /*_skyModel->indexCount*/);

	buffers.walls.bufferInfo = &buffers.local;
	buffers.walls.vertexOffset = 0;
	buffers.walls.indexOffset = buffers.walls.vertexOffset + wallVertexSize;
	buffers.walls.shadowOffset = buffers.walls.indexOffset + wallIndexSize;

	buffers.ui.bufferInfo = &buffers.local;
	buffers.ui.vertexOffset = buffers.walls.shadowOffset + buffers.walls.shadowSize;
	buffers.ui.indexOffset = buffers.ui.vertexOffset + buffers.ui.vertexSize;

	buffers.actors.bufferInfo = &buffers.local;
	buffers.actors.instanceDataOffset = buffers.ui.indexOffset + buffers.ui.indexSize;
	buffers.actors.drawInfoOffset = buffers.actors.instanceDataOffset + buffers.actors.instanceDataSize;
	buffers.actors.vertexOffset = buffers.actors.drawInfoOffset + buffers.actors.drawInfoSize;
	buffers.actors.walls.vertexOffset = buffers.actors.vertexOffset + buffers.actors.models.vertexSize;
	buffers.actors.indexOffset = buffers.actors.walls.vertexOffset + buffers.actors.walls.vertexSize;
	buffers.actors.walls.indexOffset = buffers.actors.indexOffset + buffers.actors.models.indexSize;
}

bool CreateSharedBuffer()
{
	buffers.walls.shadowStagingSize = sizeof(ShadowVertex) * buffers.walls.shadowCount * 4 +
									  sizeof(uint32_t) * buffers.walls.shadowCount * 6;

	buffers.ui.vertexStagingSize = sizeof(UiVertex) * buffers.ui.maxQuads * 4;
	buffers.ui.indexStagingSize = sizeof(uint32_t) * buffers.ui.maxQuads * 6;

	buffers.actors.instanceDataStagingSize = sizeof(ActorInstanceData) * buffers.actors.walls.count;
	if (buffers.actors.models.modelCounts.length && buffers.actors.models.loadedModelIds.length)
	{
		for (size_t i = 0; i < buffers.actors.models.loadedModelIds.length; i++)
		{
			buffers.actors.instanceDataStagingSize += sizeof(ActorInstanceData) *
													  (size_t)ListGet(buffers.actors.models.modelCounts, i);
		}
	}
	buffers.actors.drawInfoStagingSize = sizeof(VkDrawIndexedIndirectCommand) *
												 buffers.actors.models.loadedModelIds.length +
										 sizeof(VkDrawIndexedIndirectCommand) * buffers.actors.walls.count;

	buffers.actors.walls.vertexStagingSize = sizeof(ActorVertex) * buffers.actors.walls.count * 4;
	buffers.actors.walls.indexStagingSize = sizeof(uint32_t) * buffers.actors.walls.count * 6;

	buffers.shared.size = buffers.walls.shadowStagingSize +
						  buffers.ui.vertexStagingSize +
						  buffers.ui.indexStagingSize +
						  buffers.actors.instanceDataStagingSize +
						  buffers.actors.drawInfoStagingSize +
						  buffers.actors.walls.vertexStagingSize +
						  buffers.actors.walls.indexStagingSize;
	buffers.shared.memoryAllocationInfo.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
													 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	buffers.shared.memoryAllocationInfo.memoryInfo = &memoryPools.sharedMemory;
	if (!CreateBuffer(&buffers.shared, false))
	{
		return false;
	}

	if (!buffers.ui.vertices)
	{
		buffers.ui.vertices = malloc(sizeof(UiVertex) * buffers.ui.maxQuads * 4);
		CheckAlloc(buffers.ui.vertices);
	}
	if (!buffers.ui.indices)
	{
		buffers.ui.indices = malloc(sizeof(uint32_t) * buffers.ui.maxQuads * 6);
		CheckAlloc(buffers.ui.vertices);
	}


	return true;
}

void SetSharedBufferAliasingInfo()
{
	buffers.walls.stagingBufferInfo = &buffers.shared;
	buffers.walls.shadowStagingOffset = 0;

	buffers.ui.stagingBufferInfo = &buffers.shared;
	buffers.ui.vertexStagingOffset = buffers.walls.shadowStagingOffset + buffers.walls.shadowStagingSize;
	buffers.ui.indexStagingOffset = buffers.ui.vertexStagingOffset + buffers.ui.vertexStagingSize;

	buffers.actors.stagingBufferInfo = &buffers.shared;
	buffers.actors.instanceDataStagingOffset = buffers.ui.indexStagingOffset + buffers.ui.indexStagingSize;
	buffers.actors.drawInfoStagingOffset = buffers.actors.instanceDataStagingOffset +
										   buffers.actors.instanceDataStagingSize;
	buffers.actors.walls.vertexStagingOffset = buffers.actors.drawInfoStagingOffset +
											   buffers.actors.drawInfoStagingSize;
	buffers.actors.walls.indexStagingOffset = buffers.actors.walls.vertexStagingOffset +
											  buffers.actors.walls.vertexStagingSize;

	SetSharedMemoryMappingInfo();
}

// TODO: lossless
bool ResizeBuffer(Buffer *buffer, bool /*lossy*/)
{
	vkDestroyBuffer(device, buffer->buffer, NULL);
	buffer->buffer = VK_NULL_HANDLE;

	vkFreeMemory(device, buffer->memoryAllocationInfo.memoryInfo->memory, NULL);
	buffer->memoryAllocationInfo.memoryInfo->memory = VK_NULL_HANDLE;

	if (!CreateBuffer(buffer, false))
	{
		return false;
	}

	if (buffer == &buffers.shared)
	{
		buffers.walls.shadowStagingSize = sizeof(ShadowVertex) * buffers.walls.shadowCount * 4 +
										  sizeof(uint32_t) * buffers.walls.shadowCount * 6;

		buffers.ui.vertexStagingSize = sizeof(UiVertex) * buffers.ui.maxQuads * 4;
		buffers.ui.indexStagingSize = sizeof(uint32_t) * buffers.ui.maxQuads * 6;

		buffers.actors.instanceDataStagingSize = sizeof(ActorInstanceData) * buffers.actors.walls.count;
		if (buffers.actors.models.modelCounts.length && buffers.actors.models.loadedModelIds.length)
		{
			for (size_t i = 0; i < buffers.actors.models.loadedModelIds.length; i++)
			{
				buffers.actors.instanceDataStagingSize += sizeof(ActorInstanceData) *
														  (size_t)ListGet(buffers.actors.models.modelCounts, i);
			}
		}
		buffers.actors.drawInfoStagingSize = sizeof(VkDrawIndexedIndirectCommand) *
													 buffers.actors.models.loadedModelIds.length +
											 sizeof(VkDrawIndexedIndirectCommand) * buffers.actors.walls.count;

		buffers.actors.walls.vertexStagingSize = sizeof(ActorVertex) * buffers.actors.walls.count * 4;
		buffers.actors.walls.indexStagingSize = sizeof(uint32_t) * buffers.actors.walls.count * 6;

		SetSharedBufferAliasingInfo();
	}

	if (!AllocateMemory(buffer->memoryAllocationInfo.memoryInfo,
						buffer->memoryAllocationInfo.memoryRequirements.memoryTypeBits))
	{
		return false;
	}
	if (!BindMemory(buffer))
	{
		return false;
	}
	if (buffer == &buffers.shared)
	{
		return MapSharedMemory();
	}

	return true;
}

// TODO: Rewrite to allow for batched resizes
bool ResizeBufferRegion(Buffer *buffer,
						const VkDeviceSize offset,
						const VkDeviceSize oldSize,
						const VkDeviceSize newSize,
						const bool lossy)
{
	const VkDeviceSize bufferSize = buffer->size;
	void *resizedBufferData = NULL;
	void *otherBufferData = NULL;
	if (buffer->memoryAllocationInfo.memoryInfo->mappedMemory)
	{
		otherBufferData = malloc(bufferSize - oldSize);
		CheckAlloc(otherBufferData);

		memcpy(otherBufferData, buffer->memoryAllocationInfo.memoryInfo->mappedMemory, offset);
		memcpy(otherBufferData + offset,
			   buffer->memoryAllocationInfo.memoryInfo->mappedMemory + offset + oldSize,
			   bufferSize - offset - oldSize);

		if (!lossy)
		{
			resizedBufferData = malloc(oldSize);
			CheckAlloc(resizedBufferData);
			memcpy(resizedBufferData, buffer->memoryAllocationInfo.memoryInfo->mappedMemory + offset, oldSize);
		}
	} else
	{
		if (__builtin_expect(buffer->size > buffers.staging.size, false) && !ResizeStagingBuffer(buffer->size))
		{
			return false;
		}
		VkBufferCopy regions[2] = {
			{
				.size = offset,
			},
			{
				.srcOffset = offset + oldSize,
				.dstOffset = offset,
				.size = bufferSize - offset - oldSize,
			},
		};
		if (offset == 0)
		{
			regions[0] = regions[1];
			if (!CopyBuffer(buffer->buffer, buffers.staging.buffer, 1, regions))
			{
				return false;
			}
		} else if (offset + oldSize == buffer->size)
		{
			if (!CopyBuffer(buffer->buffer, buffers.staging.buffer, 1, regions))
			{
				return false;
			}
		} else
		{
			if (!CopyBuffer(buffer->buffer, buffers.staging.buffer, 2, regions))
			{
				return false;
			}
		}
	}

	buffer->size = bufferSize - oldSize + newSize;
	if (!ResizeBuffer(buffer, true))
	{
		free(resizedBufferData);
		free(otherBufferData);

		return false;
	}

	if (buffer->memoryAllocationInfo.memoryInfo->mappedMemory)
	{
		memcpy(buffer->memoryAllocationInfo.memoryInfo->mappedMemory, otherBufferData, offset);
		memcpy(buffer->memoryAllocationInfo.memoryInfo->mappedMemory + offset + newSize,
			   // ReSharper disable once CppDFANullDereference
			   otherBufferData + offset,
			   bufferSize - offset - oldSize);
		if (!lossy)
		{
			memcpy(buffer->memoryAllocationInfo.memoryInfo->mappedMemory + offset, resizedBufferData, oldSize);
		}

		free(resizedBufferData);
		free(otherBufferData);
	} else
	{
		VkBufferCopy regions[2] = {
			{
				.size = offset,
			},
			{
				.srcOffset = offset,
				.dstOffset = offset + newSize,
				.size = bufferSize - offset - oldSize,
			},
		};
		if (offset == 0)
		{
			regions[0] = regions[1];
			if (!CopyBuffer(buffers.staging.buffer, buffer->buffer, 1, regions))
			{
				return false;
			}
		} else if (offset + newSize == buffer->size)
		{
			if (!CopyBuffer(buffers.staging.buffer, buffer->buffer, 1, regions))
			{
				return false;
			}
		} else
		{
			if (!CopyBuffer(buffers.staging.buffer, buffer->buffer, 2, regions))
			{
				return false;
			}
		}
	}

	return true;
}

bool ResizeStagingBuffer(const size_t size)
{
	buffers.staging.size = size;
	if (!ResizeBuffer(&buffers.staging, true))
	{
		return false;
	}
	VulkanTest(vkMapMemory(device,
						   buffers.staging.memoryAllocationInfo.memoryInfo->memory,
						   0,
						   VK_WHOLE_SIZE,
						   0,
						   &buffers.staging.memoryAllocationInfo.memoryInfo->mappedMemory),
			   "Failed to map actor staging buffer memory!");

	return true;
}

VkResult ResizeUiBuffer()
{
	if (buffers.ui.vertexStagingOffset <= buffers.ui.indexStagingOffset &&
		buffers.ui.indexStagingOffset == buffers.ui.vertexStagingOffset + buffers.ui.vertexStagingSize)
	{
		if (!ResizeBufferRegion(&buffers.shared,
								buffers.ui.vertexStagingOffset,
								buffers.ui.vertexStagingSize + buffers.ui.indexStagingSize,
								sizeof(UiVertex) * buffers.ui.maxQuads * 4 + sizeof(uint32_t) * buffers.ui.maxQuads * 6,
								true))
		{
			return VK_ERROR_UNKNOWN;
		}
	} else if (buffers.ui.indexStagingOffset < buffers.ui.vertexStagingOffset &&
			   buffers.ui.vertexStagingOffset == buffers.ui.indexStagingOffset + buffers.ui.indexStagingSize)
	{
		if (!ResizeBufferRegion(&buffers.shared,
								buffers.ui.indexStagingOffset,
								buffers.ui.indexStagingSize + buffers.ui.vertexStagingSize,
								sizeof(UiVertex) * buffers.ui.maxQuads * 4 + sizeof(uint32_t) * buffers.ui.maxQuads * 6,
								true))
		{
			return VK_ERROR_UNKNOWN;
		}
	} else
	{
		if (!ResizeBufferRegion(&buffers.shared,
								buffers.ui.vertexStagingOffset,
								buffers.ui.vertexStagingSize,
								sizeof(UiVertex) * buffers.ui.maxQuads * 4,
								true))
		{
			return VK_ERROR_UNKNOWN;
		}
		if (!ResizeBufferRegion(&buffers.shared,
								buffers.ui.indexStagingOffset,
								buffers.ui.indexStagingSize,
								sizeof(uint32_t) * buffers.ui.maxQuads * 6,
								true))
		{
			return VK_ERROR_UNKNOWN;
		}
	}


	if (buffers.ui.vertexOffset <= buffers.ui.indexOffset &&
		buffers.ui.indexOffset == buffers.ui.vertexOffset + buffers.ui.vertexSize)
	{
		if (!ResizeBufferRegion(&buffers.local,
								buffers.ui.vertexOffset,
								buffers.ui.vertexSize + buffers.ui.indexSize,
								sizeof(UiVertex) * buffers.ui.maxQuads * 4 + sizeof(uint32_t) * buffers.ui.maxQuads * 6,
								true))
		{
			return VK_ERROR_UNKNOWN;
		}
	} else if (buffers.ui.indexOffset < buffers.ui.vertexOffset &&
			   buffers.ui.vertexOffset == buffers.ui.indexOffset + buffers.ui.indexSize)
	{
		if (!ResizeBufferRegion(&buffers.local,
								buffers.ui.indexOffset,
								buffers.ui.indexSize + buffers.ui.vertexSize,
								sizeof(UiVertex) * buffers.ui.maxQuads * 4 + sizeof(uint32_t) * buffers.ui.maxQuads * 6,
								true))
		{
			return VK_ERROR_UNKNOWN;
		}
	} else
	{
		if (!ResizeBufferRegion(&buffers.local,
								buffers.ui.vertexOffset,
								buffers.ui.vertexSize,
								sizeof(UiVertex) * buffers.ui.maxQuads * 4,
								true))
		{
			return VK_ERROR_UNKNOWN;
		}
		if (!ResizeBufferRegion(&buffers.local,
								buffers.ui.indexOffset,
								buffers.ui.indexSize,
								sizeof(uint32_t) * buffers.ui.maxQuads * 6,
								true))
		{
			return VK_ERROR_UNKNOWN;
		}
	}

	return VK_SUCCESS;
}

/// TODO: This assumes the current layout of the buffer (instanceData, vertex, index) and will not work if this is not
///  the case. I have done this to greatly simplify this function, since a function that takes multiple regions and
///  dynamically resizes them based on their layout is upcoming and will therefore be replacing this function entirely.
bool ResizeActorBuffer()
{
	const VkDeviceSize shadowStagingSize = buffers.walls.shadowStagingSize;
	VkDeviceSize newInstanceDataSize = sizeof(ActorInstanceData) * buffers.actors.walls.count;
	if (buffers.actors.models.modelCounts.length && buffers.actors.models.loadedModelIds.length)
	{
		for (size_t i = 0; i < buffers.actors.models.loadedModelIds.length; i++)
		{
			newInstanceDataSize += sizeof(ActorInstanceData) * (size_t)ListGet(buffers.actors.models.modelCounts, i);
		}
	}
	if (!ResizeBufferRegion(&buffers.shared,
							buffers.actors.instanceDataStagingOffset,
							buffers.actors.instanceDataStagingSize +
									buffers.actors.drawInfoStagingSize +
									buffers.actors.walls.vertexStagingSize +
									buffers.actors.walls.indexStagingSize,
							newInstanceDataSize +
									sizeof(VkDrawIndexedIndirectCommand) * buffers.actors.models.loadedModelIds.length +
									sizeof(VkDrawIndexedIndirectCommand) * buffers.actors.walls.count +
									sizeof(ActorVertex) * buffers.actors.walls.count * 4 +
									sizeof(uint32_t) * buffers.actors.walls.count * 6,
							true))
	{
		return false;
	}

	if (!ResizeBufferRegion(&buffers.local,
							buffers.actors.instanceDataOffset,
							buffers.actors.instanceDataSize +
									buffers.actors.drawInfoSize +
									buffers.actors.models.vertexSize +
									buffers.actors.walls.vertexSize +
									buffers.actors.models.indexSize +
									buffers.actors.walls.indexSize,
							newInstanceDataSize +
									sizeof(VkDrawIndexedIndirectCommand) * buffers.actors.models.loadedModelIds.length +
									sizeof(VkDrawIndexedIndirectCommand) * buffers.actors.walls.count +
									sizeof(ActorVertex) * buffers.actors.models.vertexCount +
									sizeof(ActorVertex) * buffers.actors.walls.count * 4 +
									sizeof(uint32_t) * buffers.actors.models.indexCount +
									sizeof(uint32_t) * buffers.actors.walls.count * 6,
							true))
	{
		return false;
	}

	if (!ResizeBufferRegion(&buffers.shared,
							buffers.walls.shadowStagingOffset,
							shadowStagingSize,
							sizeof(ShadowVertex) * buffers.walls.shadowCount * 4 +
									sizeof(uint32_t) * buffers.walls.shadowCount * 6,
							true))
	{
		return false;
	}

	if (!ResizeBufferRegion(&buffers.local,
							buffers.walls.shadowOffset,
							buffers.walls.shadowSize,
							sizeof(ShadowVertex) * buffers.walls.shadowCount * 4 +
									sizeof(uint32_t) * buffers.walls.shadowCount * 6,
							true))
	{
		return false;
	}

	buffers.walls.shadowSize = sizeof(ShadowVertex) * buffers.walls.shadowCount * 4 +
							   sizeof(uint32_t) * buffers.walls.shadowCount * 6;

	buffers.ui.vertexSize = sizeof(UiVertex) * buffers.ui.maxQuads * 4;
	buffers.ui.indexSize = sizeof(uint32_t) * buffers.ui.maxQuads * 6;

	buffers.actors.instanceDataSize = sizeof(ActorInstanceData) * buffers.actors.walls.count;
	if (buffers.actors.models.modelCounts.length && buffers.actors.models.loadedModelIds.length)
	{
		for (size_t i = 0; i < buffers.actors.models.loadedModelIds.length; i++)
		{
			buffers.actors.instanceDataSize += sizeof(ActorInstanceData) *
											   (size_t)ListGet(buffers.actors.models.modelCounts, i);
		}
	}
	buffers.actors.drawInfoSize = sizeof(VkDrawIndexedIndirectCommand) * buffers.actors.models.loadedModelIds.length +
								  sizeof(VkDrawIndexedIndirectCommand) * buffers.actors.walls.count;

	buffers.actors.models.vertexSize = sizeof(ActorVertex) * buffers.actors.models.vertexCount;
	buffers.actors.models.indexSize = sizeof(uint32_t) * buffers.actors.models.indexCount;
	buffers.actors.walls.vertexSize = sizeof(ActorVertex) * buffers.actors.walls.count * 4;
	buffers.actors.walls.indexSize = sizeof(uint32_t) * buffers.actors.walls.count * 6;
	SetLocalBufferAliasingInfo();

	return true;
}

bool LoadTexture(const Image *image)
{
	Texture *texture = calloc(1, sizeof(Texture));
	CheckAlloc(texture);
	ListAdd(&textures, texture);
	texture->imageInfo = image;

	const VkExtent3D extent = {
		.width = image->width,
		.height = image->height,
		.depth = 1,
	};
	texture->mipmapLevels = GetState()->options.mipmaps ? (uint8_t)log2(max(extent.width, extent.height)) + 1 : 1;
	if (!CreateImage(&texture->image,
					 NULL,
					 VK_FORMAT_R8G8B8A8_UNORM,
					 extent,
					 texture->mipmapLevels,
					 VK_SAMPLE_COUNT_1_BIT,
					 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					 "texture"))
	{
		return false;
	}

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

	return true;
}
