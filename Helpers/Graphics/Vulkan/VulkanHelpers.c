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
Buffers buffers = {
	.ui.vertices.allocatedSize = sizeof(UiVertex) * 4 * MAX_UI_QUADS_INIT,
	.ui.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_UI_QUADS_INIT,
	.walls.vertices.allocatedSize = sizeof(WallVertex) * 4 * MAX_WALLS_INIT,
	.walls.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_WALLS_INIT,
	.shadows.vertices.allocatedSize = sizeof(WallVertex) * 4 * MAX_WALL_ACTORS_INIT,
	.shadows.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_WALL_ACTORS_INIT,
	.wallActors.vertices.allocatedSize = sizeof(ActorVertex) * 4 * MAX_WALL_ACTORS_INIT,
	.wallActors.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_WALL_ACTORS_INIT,
	.wallActors.instanceData.allocatedSize = sizeof(ActorInstanceData) * MAX_WALL_ACTORS_INIT,
	.wallActors.drawInfo.allocatedSize = sizeof(VkDrawIndexedIndirectCommand) * MAX_WALL_ACTORS_INIT,
	.modelActors.vertices.allocatedSize = sizeof(ActorVertex) * 4 * MAX_MODEL_ACTOR_QUADS_INIT,
	.modelActors.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_MODEL_ACTOR_QUADS_INIT,
	.modelActors.instanceData.allocatedSize = sizeof(ActorInstanceData) * MAX_MODEL_ACTOR_QUADS_INIT,
	.modelActors.drawInfo.allocatedSize = sizeof(VkDrawIndexedIndirectCommand) * MAX_MODEL_ACTOR_QUADS_INIT,
};
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

	VulkanTestReturnResult(ResizeWallActorBuffers(), "Failed to resize wall actor buffers!");
	VulkanTestReturnResult(ResizeModelActorBuffers(), "Failed to resize model actor buffers!");
	LoadModelActors(level);
	LoadActorDrawInfo(level);
	ListUnlock(level->actors);
	return VK_SUCCESS;
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

void LoadRoof(const bool hasCeiling, const uint32_t ceilingTextureIndex)
{
	WallVertex *vertices = buffers.roof.vertices.data;
	uint32_t *indices = buffers.roof.indices.data;
	if (hasCeiling)
	{
		vertices[0].x = -100;
		vertices[0].y = 0.5f;
		vertices[0].z = -100;
		vertices[0].u = -100;
		vertices[0].v = -100;
		vertices[0].textureIndex = ceilingTextureIndex;

		vertices[1].x = 100;
		vertices[1].y = 0.5f;
		vertices[1].z = -100;
		vertices[1].u = 100;
		vertices[1].v = -100;
		vertices[1].textureIndex = ceilingTextureIndex;

		vertices[2].x = 100;
		vertices[2].y = 0.5f;
		vertices[2].z = 100;
		vertices[2].u = 100;
		vertices[2].v = 100;
		vertices[2].textureIndex = ceilingTextureIndex;

		vertices[3].x = -100;
		vertices[3].y = 0.5f;
		vertices[3].z = 100;
		vertices[3].u = -100;
		vertices[3].v = 100;
		vertices[3].textureIndex = ceilingTextureIndex;

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 0;
		indices[4] = 2;
		indices[5] = 3;
	} else
	{
		for (uint32_t i = 0; i < skyModel->vertexCount; i++)
		{
			// Copy {x, y, z, u, v} and discard {nx, ny, nz}
			memcpy(&vertices[i], &skyModel->vertexData[i * 8], sizeof(float) * 5);
			vertices[i].textureIndex = pushConstants.skyTextureIndex;
		}
		memcpy(indices, skyModel->indexData, sizeof(uint32_t) * skyModel->indexCount);
	}
	lunaWriteDataToBuffer(buffers.roof.vertices.buffer, vertices, buffers.roof.vertices.allocatedSize);
	lunaWriteDataToBuffer(buffers.roof.indices.buffer, indices, buffers.roof.indices.allocatedSize);
}

void LoadWalls(const Level *level)
{
	WallVertex *vertices = buffers.walls.vertices.data;
	uint32_t *indices = buffers.walls.indices.data;

	vertices[0].x = -100;
	vertices[0].y = -0.5f;
	vertices[0].z = -100;
	vertices[0].u = -100;
	vertices[0].v = -100;
	vertices[0].textureIndex = TextureIndex(level->floorTex);

	vertices[1].x = 100;
	vertices[1].y = -0.5f;
	vertices[1].z = -100;
	vertices[1].u = 100;
	vertices[1].v = -100;
	vertices[1].textureIndex = TextureIndex(level->floorTex);

	vertices[2].x = 100;
	vertices[2].y = -0.5f;
	vertices[2].z = 100;
	vertices[2].u = 100;
	vertices[2].v = 100;
	vertices[2].textureIndex = TextureIndex(level->floorTex);

	vertices[3].x = -100;
	vertices[3].y = -0.5f;
	vertices[3].z = 100;
	vertices[3].u = -100;
	vertices[3].v = 100;
	vertices[3].textureIndex = TextureIndex(level->floorTex);

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;

	for (uint32_t i = 1; i < buffers.walls.objectCount; i++)
	{
		const Wall *wall = ListGet(level->walls, i - 1);
		const float halfHeight = wall->height / 2.0f;
		const vec2 startVertex = {(float)wall->a.x, (float)wall->a.y};
		const vec2 endVertex = {(float)wall->b.x, (float)wall->b.y};
		const vec2 startUV = {wall->uvOffset, 0};
		const vec2 endUV = {(float)(wall->uvScale * wall->length + wall->uvOffset), 1};

		vertices[4 * i].x = startVertex[0];
		vertices[4 * i].y = halfHeight;
		vertices[4 * i].z = startVertex[1];
		vertices[4 * i].u = startUV[0];
		vertices[4 * i].v = startUV[1];
		vertices[4 * i].textureIndex = TextureIndex(wall->tex);
		vertices[4 * i].wallAngle = (float)wall->angle;

		vertices[4 * i + 1].x = endVertex[0];
		vertices[4 * i + 1].y = halfHeight;
		vertices[4 * i + 1].z = endVertex[1];
		vertices[4 * i + 1].u = endUV[0];
		vertices[4 * i + 1].v = startUV[1];
		vertices[4 * i + 1].textureIndex = TextureIndex(wall->tex);
		vertices[4 * i + 1].wallAngle = (float)wall->angle;

		vertices[4 * i + 2].x = endVertex[0];
		vertices[4 * i + 2].y = -halfHeight;
		vertices[4 * i + 2].z = endVertex[1];
		vertices[4 * i + 2].u = endUV[0];
		vertices[4 * i + 2].v = endUV[1];
		vertices[4 * i + 2].textureIndex = TextureIndex(wall->tex);
		vertices[4 * i + 2].wallAngle = (float)wall->angle;

		vertices[4 * i + 3].x = startVertex[0];
		vertices[4 * i + 3].y = -halfHeight;
		vertices[4 * i + 3].z = startVertex[1];
		vertices[4 * i + 3].u = startUV[0];
		vertices[4 * i + 3].v = endUV[1];
		vertices[4 * i + 3].textureIndex = TextureIndex(wall->tex);
		vertices[4 * i + 3].wallAngle = (float)wall->angle;

		indices[6 * i] = i * 4;
		indices[6 * i + 1] = i * 4 + 1;
		indices[6 * i + 2] = i * 4 + 2;
		indices[6 * i + 3] = i * 4;
		indices[6 * i + 4] = i * 4 + 2;
		indices[6 * i + 5] = i * 4 + 3;
	}
	lunaWriteDataToBuffer(buffers.walls.vertices.buffer, vertices, buffers.walls.vertices.bytesUsed);
	lunaWriteDataToBuffer(buffers.walls.indices.buffer, indices, buffers.walls.indices.bytesUsed);
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

VkResult LoadWallActors(const Level *level)
{
	if (__builtin_expect(loadedActors != level->actors.length, false))
	{
		VulkanTestReturnResult(InitActors(level), "Failed to init actors!");
	}
	ActorVertex *vertices = buffers.wallActors.vertices.data;
	uint32_t *indices = buffers.wallActors.indices.data;
	ListLock(level->actors);
	for (size_t wallCount = 0; wallCount < loadedActors; wallCount++)
	{
		const Actor *actor = ListGet(level->actors, wallCount);
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
	uint16_t *modelCounts = calloc(buffers.modelActors.loadedModelIds.length, sizeof(uint16_t));
	CheckAlloc(modelCounts);
	uint32_t *offsets = calloc(buffers.modelActors.loadedModelIds.length + 1, sizeof(uint32_t));
	CheckAlloc(offsets);
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
	free(modelCounts);
	free(offsets);
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

void LoadActorDrawInfo(const Level *level)
{
	VkDrawIndexedIndirectCommand *modelActorsDrawInfo = buffers.modelActors.drawInfo.data;
	VkDrawIndexedIndirectCommand *wallActorsDrawInfo = buffers.wallActors.drawInfo.data;
	uint32_t modelCount = 0;
	int32_t wallCount = 0;
	for (size_t i = 0; i < buffers.modelActors.loadedModelIds.length; i++)
	{
		modelActorsDrawInfo[i].indexCount = GetModelFromId((size_t)ListGet(buffers.modelActors.loadedModelIds, i))
													->indexCount;
		modelActorsDrawInfo[i].instanceCount = (size_t)ListGet(buffers.modelActors.modelCounts, i);
		modelCount += (size_t)ListGet(buffers.modelActors.modelCounts, i);
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
	if (buffers.ui.vertices.allocatedSize < buffers.ui.vertices.bytesUsed + sizeof(UiVertex) * 4 ||
		buffers.ui.indices.allocatedSize < buffers.ui.indices.bytesUsed + sizeof(uint32_t) * 6)
	{
		buffers.ui.vertices.allocatedSize += sizeof(UiVertex) * 4 * 16;
		buffers.ui.indices.allocatedSize += sizeof(uint32_t) * 6 * 16;
		buffers.ui.shouldResize = true;

		UiVertex *newVertices = realloc(buffers.ui.vertices.data, buffers.ui.vertices.allocatedSize);
		CheckAlloc(newVertices);
		buffers.ui.vertices.data = newVertices;

		uint32_t *newIndices = realloc(buffers.ui.indices.data, buffers.ui.indices.allocatedSize);
		CheckAlloc(newIndices);
		buffers.ui.indices.data = newIndices;
	}

	uint32_t *indices = buffers.ui.indices.data;
	UiVertex *vertices = buffers.ui.vertices.data;

	vertices[4 * buffers.ui.objectCount] = (UiVertex){
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
	vertices[4 * buffers.ui.objectCount + 1] = (UiVertex){
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
	vertices[4 * buffers.ui.objectCount + 2] = (UiVertex){
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
	vertices[4 * buffers.ui.objectCount + 3] = (UiVertex){
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

	indices[6 * buffers.ui.objectCount] = buffers.ui.objectCount * 4;
	indices[6 * buffers.ui.objectCount + 1] = buffers.ui.objectCount * 4 + 1;
	indices[6 * buffers.ui.objectCount + 2] = buffers.ui.objectCount * 4 + 2;
	indices[6 * buffers.ui.objectCount + 3] = buffers.ui.objectCount * 4;
	indices[6 * buffers.ui.objectCount + 4] = buffers.ui.objectCount * 4 + 2;
	indices[6 * buffers.ui.objectCount + 5] = buffers.ui.objectCount * 4 + 3;

	buffers.ui.objectCount++;
	buffers.ui.vertices.bytesUsed += sizeof(UiVertex) * 4;
	buffers.ui.indices.bytesUsed += sizeof(uint32_t) * 6;
}
