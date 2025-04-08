//
// Created by Noah on 11/23/2024.
//

#include "VulkanHelpers.h"
#include <cglm/clipspace/persp_lh_zo.h>
#include <cglm/clipspace/view_lh_zo.h>
#include <luna/luna.h>

#include "../../CommonAssets.h"
#include "../../Core/Error.h"
#include "../../Core/Logging.h"
#include "../RenderingHelpers.h"
#include "VulkanInternal.h"
#include "VulkanResources.h"

#pragma region variables
SDL_Window *vk_window = NULL;
bool minimized = false;
size_t loadedActors = 0;

VkInstance instance = VK_NULL_HANDLE;
VkSurfaceKHR surface = VK_NULL_HANDLE;
PhysicalDevice physicalDevice = {0};
QueueFamilyIndices queueFamilyIndices = {0};
VkDevice device = VK_NULL_HANDLE;
VkQueue graphicsQueue = VK_NULL_HANDLE;
VkQueue presentQueue = VK_NULL_HANDLE;
VkQueue transferQueue = VK_NULL_HANDLE;
VkSwapchainKHR swapChain = VK_NULL_HANDLE;
VkImage *swapChainImages = NULL;
uint32_t swapChainCount = 0;
VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
VkExtent2D swapChainExtent = {0};
VkImageView *swapChainImageViews = NULL;
LunaRenderPass renderPass = VK_NULL_HANDLE;
LunaDescriptorSetLayout descriptorSetLayout = LUNA_NULL_HANDLE;
VkPipelineCache pipelineCache = VK_NULL_HANDLE;
Pipelines pipelines = {.walls = VK_NULL_HANDLE, .actors = VK_NULL_HANDLE, .ui = VK_NULL_HANDLE};
VkFramebuffer *swapChainFramebuffers = NULL;
VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
VkCommandPool transferCommandPool = VK_NULL_HANDLE;
VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
bool framebufferResized = false;
uint8_t currentFrame = 0;
uint32_t swapchainImageIndex = -1;
MemoryPools memoryPools = {
	{
		.type = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	},
	{
		.type = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	},
	{
		.type = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	},
};
Buffers buffers = {0};
LunaDescriptorPool descriptorPool = LUNA_NULL_HANDLE;
LunaDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
List textures = {0};
uint32_t imageAssetIdToIndexMap[MAX_TEXTURES];
TextureSamplers textureSamplers = {
	.linearRepeat = VK_NULL_HANDLE,
	.nearestRepeat = VK_NULL_HANDLE,
	.linearNoRepeat = VK_NULL_HANDLE,
	.nearestNoRepeat = VK_NULL_HANDLE,
};
VkFormat depthImageFormat = VK_FORMAT_UNDEFINED;
VkImage depthImage = VK_NULL_HANDLE;
VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
VkImageView depthImageView = VK_NULL_HANDLE;
VkImage colorImage = VK_NULL_HANDLE;
VkDeviceMemory colorImageMemory = VK_NULL_HANDLE;
VkImageView colorImageView = VK_NULL_HANDLE;
VkClearColorValue clearColor = {{0.0f, 0.0f, 0.0f, 1.0f}};
VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
uint16_t textureCount = 0;
PushConstants pushConstants = {0};
VkCommandBuffer transferCommandBuffer = VK_NULL_HANDLE;
VkFence transferBufferFence = VK_NULL_HANDLE;
#pragma endregion variables

bool LoadActors(const Level *level)
{
	if (__builtin_expect(loadedActors == level->actors.length, true))
	{
		return true;
	}

	ListClear(&buffers.actors.models.loadedModelIds);
	ListClear(&buffers.actors.models.modelCounts);
	buffers.walls.shadowCount = 0;
	memset(&buffers.actors.models, 0, sizeof(ModelActorBuffer));
	memset(&buffers.actors.walls, 0, sizeof(WallActorBuffer));
	loadedActors = 0;
	ListLock(level->actors);
	for (size_t i = 0; i < level->actors.length; i++)
	{
		loadedActors++;
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorModel)
		{
			if (!actor->actorWall)
			{
				continue;
			}
			buffers.actors.walls.count++;
		} else
		{
			size_t index = ListFind(buffers.actors.models.loadedModelIds, (void *)actor->actorModel->id);
			if (index == -1)
			{
				index = buffers.actors.models.loadedModelIds.length;
				ListAdd(&buffers.actors.models.loadedModelIds, (void *)actor->actorModel->id);
				buffers.actors.models.vertexCount += actor->actorModel->vertexCount;
				buffers.actors.models.indexCount += actor->actorModel->indexCount;
			}
			if (index < buffers.actors.models.modelCounts.length)
			{
				buffers.actors.models.modelCounts.data[index]++;
			} else
			{
				ListAdd(&buffers.actors.models.modelCounts, (void *)1);
			}
		}
		if (actor->showShadow)
		{
			buffers.walls.shadowCount++;
		}
	}
	ListUnlock(level->actors);

	// if (!ResizeActorBuffer())
	// {
	// return false;
	// }

	ActorVertex *actorVertices = calloc(buffers.actors.models.vertexCount, sizeof(ActorVertex));
	CheckAlloc(actorVertices);
	uint32_t *actorIndices = calloc(buffers.actors.models.indexCount, sizeof(uint32_t));
	CheckAlloc(actorIndices);
	LoadActorModels(level, actorVertices, actorIndices);

	const size_t size = buffers.actors.models.vertexSize + buffers.actors.models.indexSize;
	if (__builtin_expect(size > buffers.staging.size, false)) // && !ResizeStagingBuffer(size))
	{
		return false;
	}
	void *data = buffers.staging.memoryAllocationInfo.memoryInfo->mappedMemory;

	memcpy(data, actorVertices, sizeof(ActorVertex) * buffers.actors.models.vertexCount);
	memcpy(data + buffers.actors.models.vertexSize, actorIndices, sizeof(uint32_t) * buffers.actors.models.indexCount);

	const VkBufferCopy regions[] = {
		{
			.srcOffset = 0,
			.dstOffset = buffers.actors.vertexOffset,
			.size = buffers.actors.models.vertexSize,
		},
		{
			.srcOffset = buffers.actors.models.vertexSize,
			.dstOffset = buffers.actors.indexOffset,
			.size = buffers.actors.models.indexSize,
		},
	};
	if (!CopyBuffer(buffers.staging.buffer, buffers.local.buffer, 2, regions))
	{
		return false;
	}

	free(actorVertices);
	free(actorIndices);

	return true;
}

bool CreateImageView(VkImageView *imageView,
					 const VkImage image,
					 const VkFormat format,
					 const VkImageAspectFlagBits aspectMask,
					 const uint8_t mipmapLevels,
					 const char *errorMessage)
{
	const VkComponentMapping componentMapping = {
		.r = VK_COMPONENT_SWIZZLE_IDENTITY,
		.g = VK_COMPONENT_SWIZZLE_IDENTITY,
		.b = VK_COMPONENT_SWIZZLE_IDENTITY,
		.a = VK_COMPONENT_SWIZZLE_IDENTITY,
	};
	const VkImageSubresourceRange subresourceRange = {
		.aspectMask = aspectMask,
		.baseMipLevel = 0,
		.levelCount = mipmapLevels,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};
	const VkImageViewCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.components = componentMapping,
		.subresourceRange = subresourceRange,
	};

	VulkanTest(vkCreateImageView(device, &createInfo, NULL, imageView), "%s", errorMessage);

	return true;
}

VkResult CreateShaderModule(const char *path, VkShaderModule *shaderModule)
{
	const Asset *shader = DecompressAsset(path);

	// sizeof(uint32_t) * 4 is the asset header
	VulkanTestReturnResult(lunaCreateShaderModule((uint32_t *)shader->data,
												  shader->size - sizeof(uint32_t) * 4,
												  shaderModule),
						   "Failed to create shader module!");

	return VK_SUCCESS;
}

bool CreateImage(VkImage *image,
				 VkDeviceMemory *imageMemory,
				 const VkFormat format,
				 const VkExtent3D extent,
				 const uint8_t mipmapLevels,
				 const VkSampleCountFlags samples,
				 const VkImageUsageFlags usageFlags,
				 const char *imageType)
{
	uint32_t pQueueFamilyIndices[queueFamilyIndices.familyCount];
	switch (queueFamilyIndices.familyCount)
	{
		case 1:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			break;
		case 2:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			pQueueFamilyIndices[1] = queueFamilyIndices.families & QUEUE_FAMILY_TRANSFER
											 ? queueFamilyIndices.transferFamily
											 : queueFamilyIndices.presentFamily;
			break;
		case 3:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			pQueueFamilyIndices[1] = queueFamilyIndices.presentFamily;
			pQueueFamilyIndices[2] = queueFamilyIndices.transferFamily;
			break;
		default:
			VulkanLogError("Failed to create VkSwapchainCreateInfoKHR due to invalid queueFamilyIndices!\n");
			return false;
	}

	const VkImageCreateInfo imageInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = extent,
		.mipLevels = mipmapLevels,
		.arrayLayers = 1,
		.samples = samples,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usageFlags,
		.sharingMode = queueFamilyIndices.families & QUEUE_FAMILY_PRESENTATION ? VK_SHARING_MODE_CONCURRENT
																			   : VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = queueFamilyIndices.familyCount,
		.pQueueFamilyIndices = pQueueFamilyIndices,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VulkanTest(vkCreateImage(device, &imageInfo, NULL, image), "Failed to create Vulkan %s image!", imageType);

	if (!imageMemory)
	{
		return true; // If image memory is NULL, then allocation will be handled by the calling function
	}
	// Otherwise, allocate the memory for the image

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, *image, &memoryRequirements);
	for (uint32_t i = 0; i < physicalDevice.memoryProperties.memoryTypeCount; i++)
	{
		if (memoryRequirements.memoryTypeBits & 1 << i &&
			(physicalDevice.memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			const VkDeviceSize size = memoryRequirements.alignment *
									  (VkDeviceSize)ceil((double)memoryRequirements.size /
														 (double)memoryRequirements.alignment);
			const VkMemoryAllocateInfo allocateInfo = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = NULL,
				.allocationSize = size,
				.memoryTypeIndex = i,
			};

			VulkanTest(vkAllocateMemory(device, &allocateInfo, NULL, imageMemory),
					   "Failed to allocate Vulkan %s image memory!",
					   imageType);
			break;
		}
	}

	VulkanTest(vkBindImageMemory(device, *image, *imageMemory, 0), "Failed to bind Vulkan %s image memory!", imageType);

	return true;
}

bool BeginCommandBuffer(const VkCommandBuffer *commandBuffer, const VkCommandPool commandPool)
{
	const VkCommandBufferAllocateInfo allocateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VulkanTest(vkAllocateCommandBuffers(device, &allocateInfo, (VkCommandBuffer *)commandBuffer),
			   "Failed to allocate Vulkan command buffers!");

	const VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = NULL,
	};

	VulkanTest(vkBeginCommandBuffer(*commandBuffer, &beginInfo),
			   "Failed to start the recording of Vulkan command buffers!");

	return true;
}

bool EndCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandPool commandPool, const VkQueue queue)
{
	VulkanTest(vkEndCommandBuffer(commandBuffer), "Failed to finish the recording of Vulkan command buffers!");

	const VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = NULL,
		.pWaitDstStageMask = 0,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = NULL,
	};

	VulkanTest(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE),
			   "Failed to submit Vulkan command buffers to queue!");

	VulkanTest(vkQueueWaitIdle(queue), "Failed to wait for Vulkan queue to become idle!");
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

	return true;
}

bool CreateBuffer(Buffer *buffer, const bool newAllocation)
{
	uint32_t pQueueFamilyIndices[queueFamilyIndices.familyCount];
	switch (queueFamilyIndices.familyCount)
	{
		case 1:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			break;
		case 2:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			pQueueFamilyIndices[1] = queueFamilyIndices.families & QUEUE_FAMILY_TRANSFER
											 ? queueFamilyIndices.transferFamily
											 : queueFamilyIndices.presentFamily;
			break;
		case 3:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			pQueueFamilyIndices[1] = queueFamilyIndices.presentFamily;
			pQueueFamilyIndices[2] = queueFamilyIndices.transferFamily;
			break;
		default:
			VulkanLogError("Failed to create VkSwapchainCreateInfoKHR due to invalid queueFamilyIndices!\n");
			return false;
	}

	const VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.size = buffer->size,
		.usage = buffer->memoryAllocationInfo.usageFlags,
		.sharingMode = queueFamilyIndices.families & QUEUE_FAMILY_PRESENTATION ? VK_SHARING_MODE_CONCURRENT
																			   : VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = queueFamilyIndices.familyCount,
		.pQueueFamilyIndices = pQueueFamilyIndices,
	};

	VulkanTest(vkCreateBuffer(device, &bufferInfo, NULL, &buffer->buffer), "Failed to create Vulkan buffer!");

	vkGetBufferMemoryRequirements(device, buffer->buffer, &buffer->memoryAllocationInfo.memoryRequirements);
	const VkDeviceSize memorySize = buffer->memoryAllocationInfo.memoryRequirements.alignment *
									(VkDeviceSize)
											ceil((double)buffer->memoryAllocationInfo.memoryRequirements.size /
												 (double)buffer->memoryAllocationInfo.memoryRequirements.alignment);

	buffer->memoryAllocationInfo.memoryInfo->size = memorySize;

	if (!newAllocation)
	{
		return true; // Allocation and binding will be handled elsewhere
	}

	for (uint32_t i = 0; i < physicalDevice.memoryProperties.memoryTypeCount; i++)
	{
		if (buffer->memoryAllocationInfo.memoryRequirements.memoryTypeBits & 1 << i &&
			(physicalDevice.memoryProperties.memoryTypes[i].propertyFlags &
			 buffer->memoryAllocationInfo.memoryInfo->type) == buffer->memoryAllocationInfo.memoryInfo->type)
		{
			const VkMemoryAllocateInfo allocInfo = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = NULL,
				.allocationSize = memorySize,
				.memoryTypeIndex = i,
			};

			VulkanTest(vkAllocateMemory(device, &allocInfo, NULL, &buffer->memoryAllocationInfo.memoryInfo->memory),
					   "Failed to allocate Vulkan buffer memory!");

			VulkanTest(vkBindBufferMemory(device, buffer->buffer, buffer->memoryAllocationInfo.memoryInfo->memory, 0),
					   "Failed to bind Vulkan buffer memory!");

			return true;
		}
	}

	VulkanLogError("Failed to find suitable memory type for buffer!\n");

	return false;
}

bool CopyBuffer(const VkBuffer srcBuffer,
				const VkBuffer dstBuffer,
				const uint32_t regionCount,
				const VkBufferCopy *regions)
{
	const VkCommandBuffer commandBuffer;
	if (!BeginCommandBuffer(&commandBuffer, graphicsCommandPool))
	{
		return false;
	}

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, regions);

	if (!EndCommandBuffer(commandBuffer, graphicsCommandPool, graphicsQueue))
	{
		return false;
	}

	return true;
}

inline uint32_t TextureIndex(const char *texture)
{
	return ImageIndex(LoadImage(texture));
}

inline uint32_t ImageIndex(const Image *image)
{
	const uint32_t index = imageAssetIdToIndexMap[image->id];
	if (index == -1)
	{
		if (!LoadTexture(image))
		{
			Error("Failed to load texture!");
		}
		return imageAssetIdToIndexMap[image->id];
	}
	return index;
}

void CleanupSwapChain()
{
	if (swapChainFramebuffers)
	{
		for (uint32_t i = 0; i < swapChainCount; i++)
		{
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], NULL);
		}
	}
	if (swapChainImageViews)
	{
		for (uint32_t i = 0; i < swapChainCount; i++)
		{
			vkDestroyImageView(device, swapChainImageViews[i], NULL);
		}
	}
	vkDestroySwapchainKHR(device, swapChain, NULL);

	free(swapChainImages);
	free(swapChainImageViews);
	free(swapChainFramebuffers);

	swapChainImages = NULL;
	swapChainImageViews = NULL;
	swapChainFramebuffers = NULL;
}

void CleanupColorImage()
{
	vkDestroyImageView(device, colorImageView, NULL);
	vkDestroyImage(device, colorImage, NULL);
	vkFreeMemory(device, colorImageMemory, NULL);
}

void CleanupDepthImage()
{
	vkDestroyImageView(device, depthImageView, NULL);
	vkDestroyImage(device, depthImage, NULL);
	vkFreeMemory(device, depthImageMemory, NULL);
}

void CleanupPipeline()
{
	vkDestroyPipeline(device, pipelines.walls, NULL);
	vkDestroyPipeline(device, pipelines.actors, NULL);
	vkDestroyPipeline(device, pipelines.ui, NULL);
	// vkDestroyPipelineLayout(device, pipelineLayout, NULL);
}

void CleanupSyncObjects()
{
	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);

		vkDestroyFence(device, inFlightFences[i], NULL);
	}
}

bool RecreateSwapChain()
{
	VulkanTest(vkDeviceWaitIdle(device), "Failed to wait for device to become idle!");

	CleanupSwapChain();
	CleanupColorImage();
	CleanupDepthImage();
	CleanupPipeline();
	CleanupSyncObjects();

	return CreateSwapChain() && CreateGraphicsPipelines();
}

bool DestroyBuffer(Buffer *buffer)
{
	VulkanTest(vkDeviceWaitIdle(device), "Failed to wait for device to become idle!");

	vkDestroyBuffer(device, buffer->buffer, NULL);
	buffer->buffer = VK_NULL_HANDLE;

	if (buffer->memoryAllocationInfo.memoryInfo)
	{
		vkFreeMemory(device, buffer->memoryAllocationInfo.memoryInfo->memory, NULL);
	}
	buffer->memoryAllocationInfo = (MemoryAllocationInfo){0};

	return true;
}

void LoadWalls(const Level *level,
			   const Model *skyModel,
			   WallVertex *vertices,
			   uint32_t *indices,
			   const uint32_t skyVertexCount)
{
	vertices[0 + skyVertexCount].x = -100;
	vertices[0 + skyVertexCount].y = -0.5f;
	vertices[0 + skyVertexCount].z = -100;
	vertices[0 + skyVertexCount].u = -100;
	vertices[0 + skyVertexCount].v = -100;
	vertices[0 + skyVertexCount].textureIndex = TextureIndex(level->floorTex);

	vertices[1 + skyVertexCount].x = 100;
	vertices[1 + skyVertexCount].y = -0.5f;
	vertices[1 + skyVertexCount].z = -100;
	vertices[1 + skyVertexCount].u = 100;
	vertices[1 + skyVertexCount].v = -100;
	vertices[1 + skyVertexCount].textureIndex = TextureIndex(level->floorTex);

	vertices[2 + skyVertexCount].x = 100;
	vertices[2 + skyVertexCount].y = -0.5f;
	vertices[2 + skyVertexCount].z = 100;
	vertices[2 + skyVertexCount].u = 100;
	vertices[2 + skyVertexCount].v = 100;
	vertices[2 + skyVertexCount].textureIndex = TextureIndex(level->floorTex);

	vertices[3 + skyVertexCount].x = -100;
	vertices[3 + skyVertexCount].y = -0.5f;
	vertices[3 + skyVertexCount].z = 100;
	vertices[3 + skyVertexCount].u = -100;
	vertices[3 + skyVertexCount].v = 100;
	vertices[3 + skyVertexCount].textureIndex = TextureIndex(level->floorTex);

	indices[0 + buffers.walls.skyIndexCount] = 0 + skyVertexCount;
	indices[1 + buffers.walls.skyIndexCount] = 1 + skyVertexCount;
	indices[2 + buffers.walls.skyIndexCount] = 2 + skyVertexCount;
	indices[3 + buffers.walls.skyIndexCount] = 0 + skyVertexCount;
	indices[4 + buffers.walls.skyIndexCount] = 2 + skyVertexCount;
	indices[5 + buffers.walls.skyIndexCount] = 3 + skyVertexCount;

	if (level->hasCeiling)
	{
		vertices[4].x = -100;
		vertices[4].y = 0.5f;
		vertices[4].z = -100;
		vertices[4].u = -100;
		vertices[4].v = -100;
		vertices[4].textureIndex = TextureIndex(level->ceilOrSkyTex);

		vertices[5].x = 100;
		vertices[5].y = 0.5f;
		vertices[5].z = -100;
		vertices[5].u = 100;
		vertices[5].v = -100;
		vertices[5].textureIndex = TextureIndex(level->ceilOrSkyTex);

		vertices[6].x = 100;
		vertices[6].y = 0.5f;
		vertices[6].z = 100;
		vertices[6].u = 100;
		vertices[6].v = 100;
		vertices[6].textureIndex = TextureIndex(level->ceilOrSkyTex);

		vertices[7].x = -100;
		vertices[7].y = 0.5f;
		vertices[7].z = 100;
		vertices[7].u = -100;
		vertices[7].v = 100;
		vertices[7].textureIndex = TextureIndex(level->ceilOrSkyTex);

		indices[6] = 4;
		indices[7] = 5;
		indices[8] = 6;
		indices[9] = 4;
		indices[10] = 6;
		indices[11] = 7;
	} else
	{
		for (uint32_t i = 0; i < skyModel->vertexCount; i++)
		{
			memcpy(&vertices[i], &skyModel->vertexData[i * 8], sizeof(float) * 5);
			vertices[i].textureIndex = pushConstants.skyTextureIndex;
		}
		memcpy(indices, skyModel->indexData, sizeof(uint32_t) * skyModel->indexCount);
	}

	for (uint32_t i = level->hasCeiling ? 2 : 1; i < buffers.walls.wallCount; i++)
	{
		const Wall *wall = ListGet(level->walls, i - (level->hasCeiling ? 2 : 1));
		const float halfHeight = wall->height / 2.0f;
		const vec2 startVertex = {(float)wall->a.x, (float)wall->a.y};
		const vec2 endVertex = {(float)wall->b.x, (float)wall->b.y};
		const vec2 startUV = {wall->uvOffset, 0};
		const vec2 endUV = {(float)(wall->uvScale * wall->length + wall->uvOffset), 1};

		vertices[4 * i + skyVertexCount].x = startVertex[0];
		vertices[4 * i + skyVertexCount].y = halfHeight;
		vertices[4 * i + skyVertexCount].z = startVertex[1];
		vertices[4 * i + skyVertexCount].u = startUV[0];
		vertices[4 * i + skyVertexCount].v = startUV[1];
		vertices[4 * i + skyVertexCount].textureIndex = TextureIndex(wall->tex);
		vertices[4 * i + skyVertexCount].wallAngle = (float)wall->angle;

		vertices[4 * i + 1 + skyVertexCount].x = endVertex[0];
		vertices[4 * i + 1 + skyVertexCount].y = halfHeight;
		vertices[4 * i + 1 + skyVertexCount].z = endVertex[1];
		vertices[4 * i + 1 + skyVertexCount].u = endUV[0];
		vertices[4 * i + 1 + skyVertexCount].v = startUV[1];
		vertices[4 * i + 1 + skyVertexCount].textureIndex = TextureIndex(wall->tex);
		vertices[4 * i + 1 + skyVertexCount].wallAngle = (float)wall->angle;

		vertices[4 * i + 2 + skyVertexCount].x = endVertex[0];
		vertices[4 * i + 2 + skyVertexCount].y = -halfHeight;
		vertices[4 * i + 2 + skyVertexCount].z = endVertex[1];
		vertices[4 * i + 2 + skyVertexCount].u = endUV[0];
		vertices[4 * i + 2 + skyVertexCount].v = endUV[1];
		vertices[4 * i + 2 + skyVertexCount].textureIndex = TextureIndex(wall->tex);
		vertices[4 * i + 2 + skyVertexCount].wallAngle = (float)wall->angle;

		vertices[4 * i + 3 + skyVertexCount].x = startVertex[0];
		vertices[4 * i + 3 + skyVertexCount].y = -halfHeight;
		vertices[4 * i + 3 + skyVertexCount].z = startVertex[1];
		vertices[4 * i + 3 + skyVertexCount].u = startUV[0];
		vertices[4 * i + 3 + skyVertexCount].v = endUV[1];
		vertices[4 * i + 3 + skyVertexCount].textureIndex = TextureIndex(wall->tex);
		vertices[4 * i + 3 + skyVertexCount].wallAngle = (float)wall->angle;

		indices[6 * i + buffers.walls.skyIndexCount] = i * 4 + skyVertexCount;
		indices[6 * i + 1 + buffers.walls.skyIndexCount] = i * 4 + 1 + skyVertexCount;
		indices[6 * i + 2 + buffers.walls.skyIndexCount] = i * 4 + 2 + skyVertexCount;
		indices[6 * i + 3 + buffers.walls.skyIndexCount] = i * 4 + skyVertexCount;
		indices[6 * i + 4 + buffers.walls.skyIndexCount] = i * 4 + 2 + skyVertexCount;
		indices[6 * i + 5 + buffers.walls.skyIndexCount] = i * 4 + 3 + skyVertexCount;
	}
}

void LoadActorModels(const Level *level, ActorVertex *vertices, uint32_t *indices)
{
	size_t vertexOffset = 0;
	size_t indexOffset = 0;
	ListClear(&buffers.actors.models.loadedModelIds);
	ListLock(level->actors);
	if (__builtin_expect(level->actors.length < loadedActors, false))
	{
		loadedActors = level->actors.length;
	}
	for (size_t i = 0; i < loadedActors; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorModel)
		{
			continue;
		}
		if (ListFind(buffers.actors.models.loadedModelIds, (void *)actor->actorModel->id) == -1)
		{
			ListAdd(&buffers.actors.models.loadedModelIds, (void *)actor->actorModel->id);
			const size_t vertexSize = sizeof(*vertices) * actor->actorModel->vertexCount;
			const size_t indexSize = sizeof(*indices) * actor->actorModel->indexCount;
			memcpy(&vertices[vertexOffset], actor->actorModel->vertexData, vertexSize);
			memcpy(&indices[vertexOffset], actor->actorModel->indexData, indexSize);
			vertexOffset += vertexSize;
			indexOffset += indexSize;
		}
	}
	ListUnlock(level->actors);
}

void LoadActorWalls(const Level *level, ActorVertex *vertices, uint32_t *indices)
{
	uint32_t wallCount = 0;
	ListLock(level->actors);
	if (__builtin_expect(level->actors.length < loadedActors, false))
	{
		loadedActors = level->actors.length;
	}
	for (size_t i = 0; i < loadedActors; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorWall || actor->actorModel != NULL)
		{
			continue;
		}
		const Wall *wall = actor->actorWall;
		const float halfHeight = wall->height / 2.0f;
		const vec2 startVertex = {wall->dx * 0.5f + actor->position.x, wall->dy * 0.5f + actor->position.y};
		const vec2 endVertex = {startVertex[0] - wall->dx, startVertex[1] - wall->dy};
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
}

void LoadActorInstanceData(const Level *level,
						   ActorInstanceData *instanceData,
						   ShadowVertex *shadowVertices,
						   uint32_t *shadowIndices)
{
	uint32_t wallCount = 0;
	uint32_t shadowCount = 0;
	uint16_t *modelCounts = calloc(buffers.actors.models.loadedModelIds.length, sizeof(uint16_t));
	CheckAlloc(modelCounts);
	uint32_t *offsets = calloc(buffers.actors.models.loadedModelIds.length + 1, sizeof(uint32_t));
	CheckAlloc(offsets);
	for (size_t i = 1; i <= buffers.actors.models.loadedModelIds.length; i++)
	{
		offsets[i] = offsets[i - 1] +
					 (size_t)ListGet(buffers.actors.models.modelCounts, i - 1) * sizeof(ActorInstanceData);
	}
	ListLock(level->actors);
	if (__builtin_expect(level->actors.length < loadedActors, false))
	{
		loadedActors = level->actors.length;
	}
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
			const size_t index = ListFind(buffers.actors.models.loadedModelIds, (void *)actor->actorModel->id);
			ActorInstanceData *offsetInstanceData = (void *)instanceData + offsets[index];
			memcpy(offsetInstanceData[modelCounts[index]].transform, transformMatrix, sizeof(mat4));
			offsetInstanceData[modelCounts[index]].textureIndex = TextureIndex(actor->actorModelTexture);

			modelCounts[index]++;
		} else if (actor->actorWall)
		{
			const Wall *wall = actor->actorWall;
			ActorInstanceData *offsetInstanceData = (void *)instanceData +
													offsets[buffers.actors.models.loadedModelIds.length];
			memcpy(offsetInstanceData[wallCount].transform, GLM_MAT4_IDENTITY, sizeof(mat4));
			offsetInstanceData[wallCount].textureIndex = TextureIndex(wall->tex);
			offsetInstanceData[wallCount].wallAngle = actor->actorWall->angle;

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
	free(modelCounts);
	free(offsets);
}

void LoadActorDrawInfo(const Level *level, VkDrawIndexedIndirectCommand *drawInfo)
{
	uint32_t modelCount = 0;
	uint32_t wallCount = 0;
	for (size_t i = 0; i < buffers.actors.models.loadedModelIds.length; i++)
	{
		drawInfo[i].indexCount = GetModelFromId((size_t)ListGet(buffers.actors.models.loadedModelIds, i))->indexCount;
		drawInfo[i].instanceCount = (size_t)ListGet(buffers.actors.models.modelCounts, i);
		modelCount += (size_t)ListGet(buffers.actors.models.modelCounts, i);
	}
	ListLock(level->actors);
	if (__builtin_expect(level->actors.length < loadedActors, false))
	{
		loadedActors = level->actors.length;
	}
	for (size_t i = 0; i < loadedActors; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorWall || actor->actorModel)
		{
			continue;
		}
		if (actor->actorWall)
		{
			const size_t index = wallCount + buffers.actors.models.loadedModelIds.length;
			drawInfo[index].indexCount = 6;
			drawInfo[index].instanceCount = 1;
			drawInfo[index].firstInstance = wallCount + modelCount;
			drawInfo[index].firstIndex = buffers.actors.models.indexCount + (int32_t)(wallCount * 6);
			drawInfo[index].vertexOffset = (int32_t)buffers.actors.models.vertexCount;
			wallCount++;
		}
	}
	ListUnlock(level->actors);
}

VkResult CopyBuffers(const Level *level)
{
	if (buffers.actors.models.loadedModelIds.length ||
		(buffers.actors.walls.vertexStagingSize && buffers.actors.walls.indexStagingSize))
	{
		ActorVertex *actorVertices = calloc(1, buffers.actors.walls.vertexStagingSize);
		CheckAlloc(actorVertices);
		uint32_t *actorIndices = calloc(1, buffers.actors.walls.indexStagingSize);
		CheckAlloc(actorIndices);
		LoadActorWalls(level, actorVertices, actorIndices);
		memcpy(buffers.actors.walls.vertexStaging, actorVertices, buffers.actors.walls.vertexStagingSize);
		memcpy(buffers.actors.walls.indexStaging, actorIndices, buffers.actors.walls.indexStagingSize);
		free(actorVertices);
		free(actorIndices);

		ActorInstanceData *actorInstanceData = calloc(1, buffers.actors.instanceDataStagingSize);
		CheckAlloc(actorInstanceData);
		ShadowVertex *shadowVertices = calloc(buffers.walls.shadowCount * 4, sizeof(ShadowVertex));
		CheckAlloc(shadowVertices);
		uint32_t *shadowIndices = calloc(buffers.walls.shadowCount * 6, sizeof(uint32_t));
		CheckAlloc(shadowIndices);
		LoadActorInstanceData(level, actorInstanceData, shadowVertices, shadowIndices);
		memcpy(buffers.actors.instanceDataStaging, actorInstanceData, buffers.actors.instanceDataStagingSize);
		memcpy(buffers.walls.shadowVertexStaging, shadowVertices, sizeof(ShadowVertex) * buffers.walls.shadowCount * 4);
		memcpy(buffers.walls.shadowIndexStaging, shadowIndices, sizeof(uint32_t) * buffers.walls.shadowCount * 6);
		free(actorInstanceData);
		free(shadowVertices);
		free(shadowIndices);

		VkDrawIndexedIndirectCommand *actorDrawInfo = calloc(1, buffers.actors.drawInfoStagingSize);
		CheckAlloc(actorDrawInfo);
		LoadActorDrawInfo(level, actorDrawInfo);
		memcpy(buffers.actors.drawInfoStaging, actorDrawInfo, buffers.actors.drawInfoStagingSize);
		free(actorDrawInfo);
	}

	// TODO: The transfer command buffer should be left open and multiple commands should be submitted, that way it can
	//  be handled according to if it should be or not. That will allow this function to be broken into multiple more
	//  manageable functions, and if submission is done from multiple threads it could increase performance as well.
	if (buffers.ui.quadCount > 0 ||
		buffers.walls.shadowCount ||
		(buffers.actors.models.loadedModelIds.length ||
		 (buffers.actors.walls.vertexSize && buffers.actors.walls.indexSize)))
	{
		if (buffers.walls.shadowCount)
		{
			vkCmdCopyBuffer(transferCommandBuffer,
							buffers.walls.stagingBufferInfo->buffer,
							buffers.walls.bufferInfo->buffer,
							1,
							(VkBufferCopy[]){
								{
									.srcOffset = buffers.walls.shadowStagingOffset,
									.dstOffset = buffers.walls.shadowOffset,
									.size = buffers.walls.shadowStagingSize,
								},
							});
		}
		if (buffers.actors.walls.vertexSize && buffers.actors.walls.indexSize)
		{
			vkCmdCopyBuffer(transferCommandBuffer,
							buffers.actors.stagingBufferInfo->buffer,
							buffers.actors.bufferInfo->buffer,
							4,
							(VkBufferCopy[]){
								{
									.srcOffset = buffers.actors.walls.vertexStagingOffset,
									.dstOffset = buffers.actors.walls.vertexOffset,
									.size = buffers.actors.walls.vertexStagingSize,
								},
								{
									.srcOffset = buffers.actors.walls.indexStagingOffset,
									.dstOffset = buffers.actors.walls.indexOffset,
									.size = buffers.actors.walls.indexStagingSize,
								},
								{
									.srcOffset = buffers.actors.instanceDataStagingOffset,
									.dstOffset = buffers.actors.instanceDataOffset,
									.size = buffers.actors.instanceDataStagingSize,
								},
								{
									.srcOffset = buffers.actors.drawInfoStagingOffset,
									.dstOffset = buffers.actors.drawInfoOffset,
									.size = buffers.actors.drawInfoStagingSize,
								},
							});
		} else if (buffers.actors.models.loadedModelIds.length)
		{
			vkCmdCopyBuffer(transferCommandBuffer,
							buffers.actors.stagingBufferInfo->buffer,
							buffers.actors.bufferInfo->buffer,
							2,
							(VkBufferCopy[]){
								{
									.srcOffset = buffers.actors.instanceDataStagingOffset,
									.dstOffset = buffers.actors.instanceDataOffset,
									.size = buffers.actors.instanceDataStagingSize,
								},
								{
									.srcOffset = buffers.actors.drawInfoStagingOffset,
									.dstOffset = buffers.actors.drawInfoOffset,
									.size = buffers.actors.drawInfoStagingSize,
								},
							});
		}
	}

	return VK_SUCCESS;
}

void UpdateTranslationMatrix(const Camera *camera)
{
	mat4 perspective;
	glm_perspective_lh_zo(glm_rad(camera->fov),
						  (float)swapChainExtent.width / (float)swapChainExtent.height,
						  NEAR_Z,
						  FAR_Z,
						  perspective);

	vec3 viewTarget = {cosf(camera->yaw), 0, sinf(camera->yaw)};

	// TODO roll and pitch might be messed up (test and fix as needed)
	glm_vec3_rotate(viewTarget, camera->roll, GLM_ZUP); // Roll
	glm_vec3_rotate(viewTarget, camera->pitch, GLM_XUP); // Pitch

	vec3 cameraPosition = {camera->x, camera->y, camera->z};
	glm_vec3_add(viewTarget, cameraPosition, viewTarget);

	mat4 view;
	glm_lookat_lh_zo(cameraPosition, viewTarget, (vec3){0.0f, -1.0f, 0.0f}, view);

	glm_mat4_mul(perspective, view, pushConstants.translationMatrix);
}

VkResult BeginRenderPass(const VkCommandBuffer commandBuffer, const uint32_t imageIndex)
{
	const VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = NULL,
	};

	VulkanTestReturnResult(vkBeginCommandBuffer(commandBuffer, &beginInfo),
						   "Failed to begin recording Vulkan command buffer!");

	const VkRect2D renderArea = {
		.offset = {0, 0},
		.extent = swapChainExtent,
	};
	const VkClearValue clearValues[] = {
		{.color = clearColor},
		{.depthStencil = {1, 0}},
	};
	const VkRenderPassBeginInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = NULL,
		.renderPass = renderPass,
		.framebuffer = swapChainFramebuffers[imageIndex],
		.renderArea = renderArea,
		.clearValueCount = 2,
		.pClearValues = clearValues,
	};

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	return VK_SUCCESS;
}

VkResult EndRenderPass(const VkCommandBuffer commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);

	VulkanTestReturnResult(vkEndCommandBuffer(commandBuffer), "Failed to record the Vulkan command buffer!");

	return VK_SUCCESS;
}

void DrawRectInternal(const float ndcStartX,
					  const float ndcStartY,
					  const float ndcEndX,
					  const float ndcEndY,
					  const float startU,
					  const float startV,
					  const float endU,
					  const float endV,
					  const Color color,
					  const uint32_t textureIndex)
{
	const mat4 matrix = {
		{ndcStartX, ndcStartY, startU, startV},
		{ndcEndX, ndcStartY, endU, startV},
		{ndcEndX, ndcEndY, endU, endV},
		{ndcStartX, ndcEndY, startU, endV},
	};
	DrawQuadInternal(matrix, color, textureIndex);
}

void DrawQuadInternal(const mat4 vertices_posXY_uvZW, const Color color, const uint32_t textureIndex)
{
	if (buffers.ui.quadCount >= buffers.ui.maxQuads)
	{
		buffers.ui.maxQuads += 16;
		buffers.ui.shouldResize = true;

		UiVertex *newVertices = realloc(buffers.ui.vertices, sizeof(UiVertex) * buffers.ui.maxQuads * 4);
		CheckAlloc(newVertices);
		buffers.ui.vertices = newVertices;

		uint32_t *newIndices = realloc(buffers.ui.indices, sizeof(uint32_t) * buffers.ui.maxQuads * 6);
		CheckAlloc(newIndices);
		buffers.ui.indices = newIndices;
	}

	buffers.ui.vertices[4 * buffers.ui.quadCount] = (UiVertex){
		.x = vertices_posXY_uvZW[0][0],
		.y = vertices_posXY_uvZW[0][1],
		.u = vertices_posXY_uvZW[0][2],
		.v = vertices_posXY_uvZW[0][3],
		.r = color.r,
		.g = color.g,
		.b = color.b,
		.a = color.a,
		.textureIndex = textureIndex,
	};
	buffers.ui.vertices[4 * buffers.ui.quadCount + 1] = (UiVertex){
		.x = vertices_posXY_uvZW[1][0],
		.y = vertices_posXY_uvZW[1][1],
		.u = vertices_posXY_uvZW[1][2],
		.v = vertices_posXY_uvZW[1][3],
		.r = color.r,
		.g = color.g,
		.b = color.b,
		.a = color.a,
		.textureIndex = textureIndex,
	};
	buffers.ui.vertices[4 * buffers.ui.quadCount + 2] = (UiVertex){
		.x = vertices_posXY_uvZW[2][0],
		.y = vertices_posXY_uvZW[2][1],
		.u = vertices_posXY_uvZW[2][2],
		.v = vertices_posXY_uvZW[2][3],
		.r = color.r,
		.g = color.g,
		.b = color.b,
		.a = color.a,
		.textureIndex = textureIndex,
	};
	buffers.ui.vertices[4 * buffers.ui.quadCount + 3] = (UiVertex){
		.x = vertices_posXY_uvZW[3][0],
		.y = vertices_posXY_uvZW[3][1],
		.u = vertices_posXY_uvZW[3][2],
		.v = vertices_posXY_uvZW[3][3],
		.r = color.r,
		.g = color.g,
		.b = color.b,
		.a = color.a,
		.textureIndex = textureIndex,
	};

	buffers.ui.indices[6 * buffers.ui.quadCount] = buffers.ui.quadCount * 4;
	buffers.ui.indices[6 * buffers.ui.quadCount + 1] = buffers.ui.quadCount * 4 + 1;
	buffers.ui.indices[6 * buffers.ui.quadCount + 2] = buffers.ui.quadCount * 4 + 2;
	buffers.ui.indices[6 * buffers.ui.quadCount + 3] = buffers.ui.quadCount * 4;
	buffers.ui.indices[6 * buffers.ui.quadCount + 4] = buffers.ui.quadCount * 4 + 2;
	buffers.ui.indices[6 * buffers.ui.quadCount + 5] = buffers.ui.quadCount * 4 + 3;

	buffers.ui.quadCount++;
}
