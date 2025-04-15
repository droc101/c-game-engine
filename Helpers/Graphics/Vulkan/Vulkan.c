//
// Created by Noah on 7/5/2024.
//

#include "Vulkan.h"
#include <luna/luna.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaInstance.h>
#include <string.h>

#include "../../../Structs/GlobalState.h"
#include "../../CommonAssets.h"
#include "../../Core/Error.h"
#include "../../Core/Logging.h"
#include "../../Core/MathEx.h"
#include "../../Core/Timing.h"
#include "VulkanHelpers.h"
#include "VulkanInternal.h"
#include "VulkanResources.h"

static const Level *loadedLevel;

bool VK_Init(SDL_Window *window)
{
	vk_window = window;
	// clang-format off
	if (CreateInstance() && CreateSurface() && CreateLogicalDevice() && CreateSwapChain() &&
		CreateRenderPass() && CreateDescriptorSetLayouts() &&
		CreateGraphicsPipelines() && CreateTextureSamplers() &&
		CreateBuffers() && CreateDescriptorPool() && CreateDescriptorSets())
	{
		// clang-format on

		char *vendor = calloc(32, sizeof(char));
		CheckAlloc(vendor);
		switch (physicalDevice.properties.vendorID)
		{
			case AMD:
				strncpy(vendor, "AMD", 32);
				break;
			case APPLE:
				strncpy(vendor, "Apple", 32);
				break;
			case ARM:
				strncpy(vendor, "ARM", 32);
				break;
			case IMG_TEC:
				strncpy(vendor, "ImgTec", 32);
				break;
			case INTEL:
				strncpy(vendor, "Intel", 32);
				break;
			case MESA:
				strncpy(vendor, "Mesa", 32);
				break;
			case MICROSOFT:
				strncpy(vendor, "Microsoft", 32);
				break;
			case NVIDIA:
				strncpy(vendor, "NVIDIA", 32);
				break;
			case QUALCOMM:
				strncpy(vendor, "Qualcomm", 32);
				break;
			default:
				strncpy(vendor, "Unknown", 32);
				break;
		}
		LogInfo("Vulkan Initialized\n");
		LogInfo("Vulkan Vendor: %s\n", vendor);
		LogInfo("Vulkan Device: %s\n", physicalDevice.properties.deviceName);
		LogInfo("Vulkan Version: %u.%u.%u\n",
				VK_API_VERSION_MAJOR(physicalDevice.properties.apiVersion),
				VK_API_VERSION_MINOR(physicalDevice.properties.apiVersion),
				VK_API_VERSION_PATCH(physicalDevice.properties.apiVersion));

		free(vendor);

		return true;
	}

	if (!VK_Cleanup())
	{
		VulkanLogError("Cleanup failed!");
	}

	return false;
}

VkResult VK_FrameStart()
{
	// VulkanTestReturnResult(InitActors(loadedLevel), "Failed to init actors!");

	if (minimized)
	{
		return VK_NOT_READY;
	}

	// VulkanTestReturnResult(vkWaitForFences(device, MAX_FRAMES_IN_FLIGHT, inFlightFences, VK_TRUE, UINT64_MAX),
	// 					   "Failed to wait for Vulkan fences!");
	//
	// VulkanTestReturnResult(vkResetFences(device, 1, &inFlightFences[currentFrame]), "Failed to reset Vulkan fences!");
	//
	// const VkResult acquireNextImageResult = vkAcquireNextImageKHR(device,
	// 															  swapChain,
	// 															  UINT64_MAX,
	// 															  imageAvailableSemaphores[currentFrame],
	// 															  VK_NULL_HANDLE,
	// 															  &swapchainImageIndex);
	// if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR || acquireNextImageResult == VK_SUBOPTIMAL_KHR)
	// {
	// 	if (RecreateSwapChain())
	// 	{
	// 		return acquireNextImageResult;
	// 	}
	// }
	// VulkanTestReturnResult(acquireNextImageResult, "Failed to acquire next Vulkan image index!");
	//
	// VulkanTestReturnResult(vkResetCommandBuffer(commandBuffers[currentFrame], 0),
	// 					   "Failed to reset Vulkan command buffer!");
	//
	// VulkanTestReturnResult(BeginRenderPass(commandBuffers[currentFrame], swapchainImageIndex),
	// 					   "Failed to begin render pass!");

	// vkCmdBindDescriptorSets(commandBuffers[currentFrame],
	// 						VK_PIPELINE_BIND_POINT_GRAPHICS,
	// 						pipelineLayout,
	// 						0,
	// 						1,
	// 						&descriptorSets[currentFrame],
	// 						0,
	// 						NULL);

	// VulkanTestReturnResult(vkWaitForFences(device, 1, &transferBufferFence, VK_TRUE, UINT64_MAX),
	// 					   "Failed to wait for Vulkan transfer buffer fence!");
	// VulkanTestReturnResult(vkResetFences(device, 1, &transferBufferFence),
	// 					   "Failed to reset Vulkan transfer buffer fence!");
	// const VkCommandBufferBeginInfo commandBufferBeginInfo = {
	// 	.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	// 	.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	// };
	// VulkanTestReturnResult(vkBeginCommandBuffer(transferCommandBuffer, &commandBufferBeginInfo),
	// 		   "Failed to start the recording of Vulkan transfer command buffer!");

	const LunaRenderPassBeginInfo beginInfo = {
		.renderArea.extent = swapChainExtent,
		.colorAttachmentClearValue.color = clearColor,
		.depthAttachmentClearValue.depthStencil.depth = 1,
	};
	VulkanTestReturnResult(lunaBeginRenderPass(renderPass, &beginInfo), "Failed to begin render pass!");

	const LunaGraphicsPipelineBindInfo bindInfo = {
		.descriptorSetCount = 1,
		.descriptorSets = &descriptorSets[currentFrame],
	};
	lunaBindDescriptorSets(pipelines.ui, &bindInfo);

	buffers.ui.objectCount = 0;
	buffers.ui.vertices.bytesUsed = 0;
	buffers.ui.indices.bytesUsed = 0;

	return VK_SUCCESS;
}

VkResult VK_FrameEnd()
{
	if (buffers.ui.shouldResize)
	{
		lunaDestroyBuffer(buffers.ui.vertices.buffer);
		lunaDestroyBuffer(buffers.ui.indices.buffer);

		VulkanTestReturnResult(CreateUiBuffers(), "Failed to recreate UI buffers!");
	}

	if (buffers.ui.objectCount > 0)
	{
		lunaWriteDataToBuffer(buffers.ui.vertices.buffer, buffers.ui.vertices.data, buffers.ui.vertices.bytesUsed);
		lunaWriteDataToBuffer(buffers.ui.indices.buffer, buffers.ui.indices.data, buffers.ui.indices.bytesUsed);
	}

	// VulkanTestReturnResult(vkEndCommandBuffer(transferCommandBuffer),
	// 		   "Failed to finish the recording of Vulkan transfer command buffer!");
	//
	// const VkSubmitInfo queueSubmitInfo = {
	// 	.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	// 	.commandBufferCount = 1,
	// 	.pCommandBuffers = &transferCommandBuffer,
	// };
	//
	// VulkanTestReturnResult(vkQueueSubmit(transferQueue, 1, &queueSubmitInfo, transferBufferFence),
	// 		   "Failed to submit Vulkan transfer command buffer to queue!");

	lunaNextSubpass();
	if (buffers.ui.objectCount > 0)
	{
		VulkanTestReturnResult(lunaDrawBufferIndexed(buffers.ui.vertices.buffer,
													 buffers.ui.indices.buffer,
													 0,
													 VK_INDEX_TYPE_UINT32,
													 pipelines.ui,
													 (LunaGraphicsPipelineBindInfo[]){0},
													 buffers.ui.objectCount * 6,
													 1,
													 0,
													 0,
													 0),
							   "Failed to draw UI!");
	}

	lunaEndRenderPass();
	VulkanTestReturnResult(lunaPresentSwapChain(), "Failed to present swap chain!");

	// const VkSubmitInfo submitInfo = {
	// 	.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	// 	.pNext = NULL,
	// 	.waitSemaphoreCount = 1,
	// 	.pWaitSemaphores = &imageAvailableSemaphores[currentFrame],
	// 	.pWaitDstStageMask = (VkPipelineStageFlags[]){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
	// 	.commandBufferCount = 1,
	// 	.pCommandBuffers = &commandBuffers[currentFrame],
	// 	.signalSemaphoreCount = 1,
	// 	.pSignalSemaphores = &renderFinishedSemaphores[currentFrame],
	// };
	// VulkanTestReturnResult(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]),
	// 					   "Failed to submit Vulkan draw command buffer!");
	//
	// const VkSwapchainKHR swapChains[] = {swapChain};
	// const VkPresentInfoKHR presentInfo = {
	// 	.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	// 	.pNext = NULL,
	// 	.waitSemaphoreCount = 1,
	// 	.pWaitSemaphores = &renderFinishedSemaphores[currentFrame],
	// 	.swapchainCount = 1,
	// 	.pSwapchains = swapChains,
	// 	.pImageIndices = &swapchainImageIndex,
	// 	.pResults = NULL,
	// };
	// const VkResult queuePresentResult = vkQueuePresentKHR(presentQueue, &presentInfo);
	// if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || queuePresentResult == VK_SUBOPTIMAL_KHR)
	// {
	// 	if (RecreateSwapChain())
	// 	{
	// 		return queuePresentResult;
	// 	}
	// }
	// VulkanTestReturnResult(queuePresentResult, "Failed to queue frame for presentation!");
	//
	// currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return VK_SUCCESS;
}

VkResult VK_RenderLevel(const Level *level, const Camera *camera)
{
	if (loadedLevel != level)
	{
		if (!VK_LoadLevelWalls(level))
		{
			return VK_ERROR_UNKNOWN;
		}
	}
	pushConstants.position[0] = (float)loadedLevel->player.pos.x;
	pushConstants.position[1] = (float)loadedLevel->player.pos.y;
	pushConstants.yaw = camera->yaw + 1.5f * PIf;
	UpdateTranslationMatrix(camera);

	VulkanTestReturnResult(LoadWallActors(loadedLevel), "Failed to load wall actors!");
	// LoadActorDrawInfo(loadedLevel);
	VulkanTestReturnResult(UpdateActorData(loadedLevel), "Failed to update actor data!");

	VulkanTestReturnResult(lunaPushConstants(pipelines.walls), "Failed to push constants!");

	if (buffers.walls.objectCount || buffers.shadows.objectCount)
	{
		lunaBindVertexBuffers(0,
							  2,
							  (LunaBuffer[]){buffers.shadows.vertices.buffer, buffers.walls.vertices.buffer},
							  (VkDeviceSize[]){0, 0});
	}
	if (buffers.walls.objectCount)
	{
		// TODO: This draws the floor, but uses the wall buffer
		VulkanTestReturnResult(lunaDrawBufferIndexed(NULL,
													 buffers.walls.indices.buffer,
													 0,
													 VK_INDEX_TYPE_UINT32,
													 pipelines.walls,
													 (LunaGraphicsPipelineBindInfo[]){0},
													 6,
													 1,
													 0,
													 0,
													 0),
							   "Failed to draw floor!");
	}
	// if (buffers.shadows.objectCount)
	// {
	// 	// 0x53484457 is "SHDW", to encode that we are drawing the shadows
	// 	VulkanTestReturnResult(lunaDrawBufferIndexed(NULL,
	// 												 buffers.shadows.indices.buffer,
	// 												 0,
	// 												 VK_INDEX_TYPE_UINT32,
	// 												 pipelines.walls,
	// 												 (LunaGraphicsPipelineBindInfo[]){0},
	// 												 buffers.shadows.objectCount * 6,
	// 												 1,
	// 												 0,
	// 												 0,
	// 												 0x53484457),
	// 						   "Failed to draw shadows!");
	// }
	if (buffers.walls.objectCount > loadedLevel->hasCeiling + 1)
	{
		// 0x57414C4C is "WALL", to encode that we are drawing the walls
		VulkanTestReturnResult(lunaDrawBufferIndexed(buffers.walls.vertices.buffer,
													 buffers.walls.indices.buffer,
													 sizeof(uint32_t) * 6,
													 VK_INDEX_TYPE_UINT32,
													 pipelines.walls,
													 (LunaGraphicsPipelineBindInfo[]){0},
													 (buffers.walls.objectCount - 1) * 6,
													 1,
													 0,
													 0,
													 0x57414C4C),
							   "Failed to draw walls!");
	}

	lunaBindVertexBuffers(0,
						  2,
						  (LunaBuffer[]){buffers.roof.vertices.buffer, buffers.roof.vertices.buffer},
						  (VkDeviceSize[]){0, 0});
	VulkanTestReturnResult(lunaDrawBufferIndexed(NULL,
												 buffers.roof.indices.buffer,
												 0,
												 VK_INDEX_TYPE_UINT32,
												 pipelines.walls,
												 (LunaGraphicsPipelineBindInfo[]){0},
												 level->hasCeiling ? 6 : skyModel->indexCount,
												 1,
												 0,
												 0,
												 0),
						   "Failed to draw roof!");

	if (buffers.wallActors.count)
	{
		lunaBindVertexBuffers(0,
							  2,
							  (LunaBuffer[]){buffers.wallActors.vertices.buffer,
											 buffers.wallActors.instanceData.buffer},
							  (VkDeviceSize[]){0, 0});

		VulkanTestReturnResult(lunaDrawBufferIndexedIndirect(NULL,
															 buffers.wallActors.indices.buffer,
															 0,
															 VK_INDEX_TYPE_UINT32,
															 pipelines.actors,
															 (LunaGraphicsPipelineBindInfo[]){0},
															 buffers.wallActors.drawInfo.buffer,
															 0,
															 buffers.wallActors.count,
															 sizeof(VkDrawIndexedIndirectCommand)),
							   "Failed to draw wall actors!");
	}
	if (buffers.modelActors.loadedModelIds.length)
	{
		lunaBindVertexBuffers(0,
							  2,
							  (LunaBuffer[]){buffers.modelActors.vertices.buffer,
											 buffers.modelActors.instanceData.buffer},
							  (VkDeviceSize[]){0, 0});

		VulkanTestReturnResult(lunaDrawBufferIndexedIndirect(NULL,
															 buffers.modelActors.indices.buffer,
															 0,
															 VK_INDEX_TYPE_UINT32,
															 pipelines.actors,
															 (LunaGraphicsPipelineBindInfo[]){0},
															 buffers.modelActors.drawInfo.buffer,
															 0,
															 buffers.modelActors.loadedModelIds.length,
															 sizeof(VkDrawIndexedIndirectCommand)),
							   "Failed to draw model actors!");
	}

	return VK_SUCCESS;
}

bool VK_Cleanup()
{
	VulkanTest(lunaDestroyInstance(), "Cleanup failed!");
	free(buffers.ui.vertices.data);
	free(buffers.ui.indices.data);
	free(buffers.roof.vertices.data);
	free(buffers.roof.indices.data);
	free(buffers.walls.vertices.data);
	free(buffers.walls.indices.data);
	free(buffers.shadows.vertices.data);
	free(buffers.shadows.indices.data);
	free(buffers.wallActors.vertices.data);
	free(buffers.wallActors.indices.data);
	free(buffers.wallActors.instanceData.data);
	free(buffers.wallActors.drawInfo.data);
	free(buffers.modelActors.vertices.data);
	free(buffers.modelActors.indices.data);
	free(buffers.modelActors.instanceData.data);
	free(buffers.modelActors.drawInfo.data);

	return true;
}

inline void VK_Minimize()
{
	minimized = true;
}

inline void VK_Restore()
{
	minimized = false;
}

inline uint8_t VK_GetSampleCountFlags()
{
	return physicalDevice.properties.limits.framebufferColorSampleCounts &
		   physicalDevice.properties.limits.framebufferDepthSampleCounts &
		   0xF;
}

bool VK_LoadLevelWalls(const Level *level)
{
	if (level->hasCeiling)
	{
		pushConstants.skyVertexCount = 0;
		pushConstants.skyTextureIndex = MAX_TEXTURES;
	} else
	{
		pushConstants.skyVertexCount = skyModel->vertexCount;
		pushConstants.skyTextureIndex = TextureIndex(level->ceilOrSkyTex);
	}
	pushConstants.shadowTextureIndex = TextureIndex(TEXTURE("vfx_shadow"));
	pushConstants.fogStart = (float)level->fogStart;
	pushConstants.fogEnd = (float)level->fogEnd;
	pushConstants.fogColor = level->fogColor;

	LoadRoof(level->hasCeiling, TextureIndex(level->ceilOrSkyTex));

	buffers.walls.objectCount = level->walls.length + 1;
	buffers.walls.vertices.bytesUsed = sizeof(WallVertex) * 4 * buffers.walls.objectCount;
	buffers.walls.indices.bytesUsed = sizeof(uint32_t) * 6 * buffers.walls.objectCount;
	VulkanTest(ResizeWallBuffers(), "Failed to resize wall buffers!");
	LoadWalls(level);

	loadedLevel = level;

	// free(wallVertices);
	// free(wallIndices);

	loadedActors = 0;
	VulkanTest(InitActors(level), "Failed to load actors!");
	return true;
}

void VK_DrawColoredQuad(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const Color color)
{
	DrawRectInternal(VK_X_TO_NDC(x), VK_Y_TO_NDC(y), VK_X_TO_NDC(x + w), VK_Y_TO_NDC(y + h), 0, 0, 0, 0, color, -1);
}

void VK_DrawColoredQuadsBatched(const float *vertices, const int32_t quadCount, const Color color)
{
	for (int32_t i = 0; i < quadCount; i++)
	{
		const uint32_t index = i * 8;
		const mat4 matrix = {
			{vertices[index + 0], vertices[index + 1], 0, 0},
			{vertices[index + 2], vertices[index + 3], 0, 0},
			{vertices[index + 4], vertices[index + 5], 0, 0},
			{vertices[index + 6], vertices[index + 7], 0, 0},
		};
		DrawQuadInternal(matrix, color, -1);
	}
}

void VK_DrawTexturedQuad(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const char *texture)
{
	DrawRectInternal(VK_X_TO_NDC(x),
					 VK_Y_TO_NDC(y),
					 VK_X_TO_NDC(x + w),
					 VK_Y_TO_NDC(y + h),
					 0,
					 0,
					 1,
					 1,
					 COLOR_WHITE,
					 TextureIndex(texture));
}

void VK_DrawTexturedQuadMod(const int32_t x,
							const int32_t y,
							const int32_t w,
							const int32_t h,
							const char *texture,
							const Color color)
{
	DrawRectInternal(VK_X_TO_NDC(x),
					 VK_Y_TO_NDC(y),
					 VK_X_TO_NDC(x + w),
					 VK_Y_TO_NDC(y + h),
					 0,
					 0,
					 1,
					 1,
					 color,
					 TextureIndex(texture));
}

void VK_DrawTexturedQuadRegion(const int32_t x,
							   const int32_t y,
							   const int32_t w,
							   const int32_t h,
							   const int32_t regionX,
							   const int32_t regionY,
							   const int32_t regionW,
							   const int32_t regionH,
							   const char *texture)
{
	const Image *image = LoadImage(texture);

	const float startU = (float)regionX / (float)image->width;
	const float startV = (float)regionY / (float)image->height;

	DrawRectInternal(VK_X_TO_NDC(x),
					 VK_Y_TO_NDC(y),
					 VK_X_TO_NDC(x + w),
					 VK_Y_TO_NDC(y + h),
					 startU,
					 startV,
					 startU + (float)regionW / (float)image->width,
					 startV + (float)regionH / (float)image->height,
					 COLOR_WHITE,
					 ImageIndex(image));
}

void VK_DrawTexturedQuadRegionMod(const int32_t x,
								  const int32_t y,
								  const int32_t w,
								  const int32_t h,
								  const int32_t regionX,
								  const int32_t regionY,
								  const int32_t regionW,
								  const int32_t regionH,
								  const char *texture,
								  const Color color)
{
	const Image *image = LoadImage(texture);

	const float startU = (float)regionX / (float)image->width;
	const float startV = (float)regionY / (float)image->height;

	DrawRectInternal(VK_X_TO_NDC(x),
					 VK_Y_TO_NDC(y),
					 VK_X_TO_NDC(x + w),
					 VK_Y_TO_NDC(y + h),
					 startU,
					 startV,
					 startU + (float)regionW / (float)image->width,
					 startV + (float)regionH / (float)image->height,
					 color,
					 ImageIndex(image));
}

void VK_DrawTexturedQuadsBatched(const float *vertices, const int32_t quadCount, const char *texture, const Color color)
{
	for (int32_t i = 0; i < quadCount; i++)
	{
		const uint32_t index = i * 16;
		const mat4 matrix = {
			{
				vertices[index + 0],
				vertices[index + 1],
				vertices[index + 2],
				vertices[index + 3],
			},
			{
				vertices[index + 4],
				vertices[index + 5],
				vertices[index + 6],
				vertices[index + 7],
			},
			{
				vertices[index + 8],
				vertices[index + 9],
				vertices[index + 10],
				vertices[index + 11],
			},
			{
				vertices[index + 12],
				vertices[index + 13],
				vertices[index + 14],
				vertices[index + 15],
			},
		};
		DrawQuadInternal(matrix, color, TextureIndex(texture));
	}
}

void VK_DrawLine(const int32_t startX,
				 const int32_t startY,
				 const int32_t endX,
				 const int32_t endY,
				 const int32_t thickness,
				 const Color color)
{
	const float dx = (float)endX - (float)startX;
	const float dy = (float)endY - (float)startY;
	const float distance = 2.0f * sqrtf(dx * dx + dy * dy);

	const mat4 matrix = {
		{
			VK_X_TO_NDC(-thickness * dy / distance + (float)startX),
			VK_Y_TO_NDC(thickness * dx / distance + (float)startY),
			0,
			0,
		},
		{
			VK_X_TO_NDC(-thickness * dy / distance + (float)endX),
			VK_Y_TO_NDC(thickness * dx / distance + (float)endY),
			0,
			0,
		},
		{
			VK_X_TO_NDC(thickness * dy / distance + (float)endX),
			VK_Y_TO_NDC(-thickness * dx / distance + (float)endY),
			0,
			0,
		},
		{
			VK_X_TO_NDC(thickness * dy / distance + (float)startX),
			VK_Y_TO_NDC(-thickness * dx / distance + (float)startY),
			0,
			0,
		},
	};
	DrawQuadInternal(matrix, color, -1);
}

void VK_DrawRectOutline(const int32_t x,
						const int32_t y,
						const int32_t w,
						const int32_t h,
						const int32_t thickness,
						const Color color)
{
	VK_DrawLine(x, y, x + w, y, thickness, color);
	VK_DrawLine(x + w, y, x + w, y + h, thickness, color);
	VK_DrawLine(x + w, y + h, x, y + h, thickness, color);
	VK_DrawLine(x, y + h, x, y, thickness, color);
}

void VK_ClearColor(const Color color)
{
	clearColor = (VkClearColorValue){{color.r, color.g, color.b, color.a}};
}

void VK_SetTexParams(const char *texture, const bool linear, const bool repeat)
{
	const uint32_t textureIndex = TextureIndex(texture);
	LunaDescriptorImageInfo imageInfo = {
		.image = textures.data[textureIndex],
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};
	if (linear && repeat)
	{
		imageInfo.sampler = textureSamplers.linearRepeat;
	} else if (linear)
	{
		imageInfo.sampler = textureSamplers.linearNoRepeat;
	} else if (repeat)
	{
		imageInfo.sampler = textureSamplers.nearestRepeat;
	} else
	{
		imageInfo.sampler = textureSamplers.nearestNoRepeat;
	}
	LunaWriteDescriptorSet writeDescriptors[MAX_FRAMES_IN_FLIGHT];
	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		writeDescriptors[i] = (LunaWriteDescriptorSet){
			.descriptorSet = descriptorSets[i],
			.bindingName = "Textures",
			.descriptorArrayElement = textureIndex,
			.descriptorCount = 1,
			.imageInfo = &imageInfo,
		};
		lunaWriteDescriptorSets(MAX_FRAMES_IN_FLIGHT, writeDescriptors);
	}
}
