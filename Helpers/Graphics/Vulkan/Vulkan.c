//
// Created by Noah on 7/5/2024.
//

#include "Vulkan.h"
#include <assert.h>
#include <luna/luna.h>
#include <SDL_vulkan.h>
#include <string.h>
#include "../../CommonAssets.h"
#include "../../Core/LodThread.h"
#include "../../Core/MathEx.h"
#include "VulkanActors.h"
#include "VulkanHelpers.h"
#include "VulkanInternal.h"
#include "VulkanResources.h"

static const Level *loadedLevel;
static uint8_t currentFrame;

bool VK_Init(SDL_Window *window)
{
	// clang-format off
	if (CreateInstance(window) && CreateSurface() && CreateLogicalDevice() && CreateSwapchain() && CreateRenderPass() &&
		CreateDescriptorSetLayouts() && CreateGraphicsPipelines() && CreateTextureSamplers() &&
		CreateDescriptorSets() && CreateBuffers())
	{
		// clang-format on

		VulkanActorsVariablesInit();

		const VkPhysicalDeviceProperties physicalDeviceProperties = lunaGetPhysicalDeviceProperties();
		char vendor[32] = {};
		switch (physicalDeviceProperties.vendorID)
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
		LogInfo("Vulkan Device: %s\n", physicalDeviceProperties.deviceName);
		LogInfo("Vulkan Version: %u.%u.%u\n",
				VK_API_VERSION_MAJOR(physicalDeviceProperties.apiVersion),
				VK_API_VERSION_MINOR(physicalDeviceProperties.apiVersion),
				VK_API_VERSION_PATCH(physicalDeviceProperties.apiVersion));

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
	if (minimized)
	{
		return VK_NOT_READY;
	}

	if (LockLodThreadMutex() != 0)
	{
		LogError("Failed to lock LOD thread mutex with error: %s", SDL_GetError());
		return VK_ERROR_UNKNOWN;
	}

	const LunaRenderPassBeginInfo beginInfo = {
		.renderArea.extent = swapChainExtent,
		.depthAttachmentClearValue.depthStencil.depth = 1,
	};
	VulkanTestResizeSwapchain(lunaBeginRenderPass(renderPass, &beginInfo), "Failed to begin render pass!");

	if (UnlockLodThreadMutex() != 0)
	{
		LogError("Failed to unlock LOD thread mutex with error: %s", SDL_GetError());
		return VK_ERROR_UNKNOWN;
	}

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

		const LunaBufferCreationInfo vertexBufferCreationInfo = {
			.size = buffers.ui.vertices.allocatedSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&vertexBufferCreationInfo, &buffers.ui.vertices.buffer),
							   "Failed to recreate UI vertex buffer!");

		const LunaBufferCreationInfo indexBufferCreationInfo = {
			.size = buffers.ui.indices.allocatedSize,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&indexBufferCreationInfo, &buffers.ui.indices.buffer),
							   "Failed to recreate UI index buffer!");
	}
	if (buffers.ui.objectCount > 0)
	{
		lunaWriteDataToBuffer(buffers.ui.vertices.buffer, buffers.ui.vertices.data, buffers.ui.vertices.bytesUsed, 0);
		lunaWriteDataToBuffer(buffers.ui.indices.buffer, buffers.ui.indices.data, buffers.ui.indices.bytesUsed, 0);
	}

	if (LockLodThreadMutex() != 0)
	{
		LogError("Failed to lock LOD thread mutex with error: %s", SDL_GetError());
		return VK_ERROR_UNKNOWN;
	}

	if (buffers.ui.objectCount > 0)
	{
		const VkViewport viewport = {
			.width = (float)swapChainExtent.width,
			.height = (float)swapChainExtent.height,
			.maxDepth = 1,
		};
		const LunaViewportBindInfo viewportBindInfo = {
			.viewportCount = 1,
			.viewports = &viewport,
		};
		const VkRect2D scissor = {
			.extent = swapChainExtent,
		};
		const LunaScissorBindInfo scissorBindInfo = {
			.scissorCount = 1,
			.scissors = &scissor,
		};
		const LunaDynamicStateBindInfo dynamicStateBindInfos[] = {
			{
				.dynamicStateType = VK_DYNAMIC_STATE_VIEWPORT,
				.viewportBindInfo = &viewportBindInfo,
			},
			{
				.dynamicStateType = VK_DYNAMIC_STATE_SCISSOR,
				.scissorBindInfo = &scissorBindInfo,
			},
		};
		const LunaGraphicsPipelineBindInfo pipelineBindInfo = {
			.descriptorSetCount = 1,
			.descriptorSets = &descriptorSets[currentFrame],
			.dynamicStateCount = sizeof(dynamicStateBindInfos) / sizeof(*dynamicStateBindInfos),
			.dynamicStates = dynamicStateBindInfos,
		};
		VulkanTestReturnResult(lunaDrawBufferIndexed(buffers.ui.vertices.buffer,
													 buffers.ui.indices.buffer,
													 0,
													 VK_INDEX_TYPE_UINT32,
													 pipelines.ui,
													 &pipelineBindInfo,
													 buffers.ui.objectCount * 6,
													 1,
													 0,
													 0,
													 0),
							   "Failed to draw UI!");
	}

	lunaEndRenderPass();

	VulkanTestResizeSwapchain(lunaPresentSwapchain(), "Failed to present swapchain!");
	if (UnlockLodThreadMutex() != 0)
	{
		LogError("Failed to unlock LOD thread mutex with error: %s", SDL_GetError());
		return VK_ERROR_UNKNOWN;
	}
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return VK_SUCCESS;
}

VkResult VK_RenderLevel(const Level *level, const Camera *camera, const Viewmodel *viewmodel)
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
	UpdateTransformMatrix(camera);

	if (LockLodThreadMutex() != 0)
	{
		LogError("Failed to lock LOD thread mutex with error: %s", SDL_GetError());
		return VK_ERROR_UNKNOWN;
	}

	VulkanTestReturnResult(lunaPushConstants(pipelines.walls), "Failed to push constants!");

	const VkViewport viewport = {
		.width = (float)swapChainExtent.width,
		.height = (float)swapChainExtent.height,
		.maxDepth = 1,
	};
	const LunaViewportBindInfo viewportBindInfo = {
		.viewportCount = 1,
		.viewports = &viewport,
	};
	const VkRect2D scissor = {
		.extent = swapChainExtent,
	};
	const LunaScissorBindInfo scissorBindInfo = {
		.scissorCount = 1,
		.scissors = &scissor,
	};
	const LunaDynamicStateBindInfo dynamicStateBindInfos[] = {
		{
			.dynamicStateType = VK_DYNAMIC_STATE_VIEWPORT,
			.viewportBindInfo = &viewportBindInfo,
		},
		{
			.dynamicStateType = VK_DYNAMIC_STATE_SCISSOR,
			.scissorBindInfo = &scissorBindInfo,
		},
	};
	const LunaGraphicsPipelineBindInfo pipelineBindInfo = {
		.descriptorSetCount = 1,
		.descriptorSets = &descriptorSets[currentFrame],
		.dynamicStateCount = sizeof(dynamicStateBindInfos) / sizeof(*dynamicStateBindInfos),
		.dynamicStates = dynamicStateBindInfos,
	};

	if (level->hasCeiling)
	{
		VulkanTestReturnResult(lunaDrawBuffer(NULL, pipelines.floorAndCeiling, &pipelineBindInfo, 12, 1, 0, 0),
							   "Failed to draw floor and ceiling!");
	} else
	{
		VulkanTestReturnResult(lunaDrawBufferIndexed(buffers.sky.vertices.buffer,
													 buffers.sky.indices.buffer,
													 0,
													 VK_INDEX_TYPE_UINT32,
													 pipelines.sky,
													 &pipelineBindInfo,
													 buffers.sky.objectCount,
													 1,
													 0,
													 0,
													 0),
							   "Failed to draw sky!");
		VulkanTestReturnResult(lunaDrawBuffer(NULL, pipelines.floorAndCeiling, &pipelineBindInfo, 6, 1, 0, 0),
							   "Failed to draw floor!");
	}

	if (buffers.walls.objectCount)
	{
		VulkanTestReturnResult(lunaDrawBufferIndexed(buffers.walls.vertices.buffer,
													 buffers.walls.indices.buffer,
													 0,
													 VK_INDEX_TYPE_UINT32,
													 pipelines.walls,
													 &pipelineBindInfo,
													 buffers.walls.objectCount * 6,
													 1,
													 0,
													 0,
													 0),
							   "Failed to draw walls!");
	}

	if (buffers.actorWalls.count)
	{
		lunaBindVertexBuffers(0,
							  2,
							  (LunaBuffer[]){buffers.actorWalls.vertices.buffer,
											 buffers.actorWalls.instanceData.buffer},
							  (VkDeviceSize[]){0, 0});

		VulkanTestReturnResult(lunaDrawBufferIndexedIndirect(NULL,
															 buffers.actorWalls.indices.buffer,
															 0,
															 VK_INDEX_TYPE_UINT32,
															 pipelines.actorWalls,
															 &pipelineBindInfo,
															 buffers.actorWalls.drawInfo.buffer,
															 0,
															 buffers.actorWalls.count,
															 sizeof(VkDrawIndexedIndirectCommand)),
							   "Failed to draw wall actors!");
	}

	if (buffers.actorModels.shadedDrawInfo.bytesUsed)
	{
		lunaBindVertexBuffers(0,
							  2,
							  (LunaBuffer[]){buffers.actorModels.vertices.buffer,
											 buffers.actorModels.instanceData.buffer},
							  (VkDeviceSize[]){0, 0});

		VulkanTestReturnResult(lunaDrawBufferIndexedIndirect(NULL,
															 buffers.actorModels.indices.buffer,
															 0,
															 VK_INDEX_TYPE_UINT32,
															 pipelines.shadedActorModels,
															 &pipelineBindInfo,
															 buffers.actorModels.shadedDrawInfo.buffer,
															 0,
															 buffers.actorModels.shadedDrawInfo.bytesUsed /
																	 sizeof(VkDrawIndexedIndirectCommand),
															 sizeof(VkDrawIndexedIndirectCommand)),
							   "Failed to draw shaded model actors!");
	}
	if (buffers.actorModels.unshadedDrawInfo.bytesUsed)
	{
		lunaBindVertexBuffers(0,
							  2,
							  (LunaBuffer[]){buffers.actorModels.vertices.buffer,
											 buffers.actorModels.instanceData.buffer},
							  (VkDeviceSize[]){0, 0});

		VulkanTestReturnResult(lunaDrawBufferIndexedIndirect(NULL,
															 buffers.actorModels.indices.buffer,
															 0,
															 VK_INDEX_TYPE_UINT32,
															 pipelines.unshadedActorModels,
															 &pipelineBindInfo,
															 buffers.actorModels.unshadedDrawInfo.buffer,
															 0,
															 buffers.actorModels.unshadedDrawInfo.bytesUsed /
																	 sizeof(VkDrawIndexedIndirectCommand),
															 sizeof(VkDrawIndexedIndirectCommand)),
							   "Failed to draw unshaded model actors!");
	}

	if (viewmodel->enabled)
	{
		UpdateViewModelMatrix(viewmodel);
		lunaBindVertexBuffers(0,
							  2,
							  (LunaBuffer[]){buffers.viewModel.vertices, buffers.viewModel.instanceDataBuffer},
							  (VkDeviceSize[]){0, 0});
		VulkanTestReturnResult(lunaDrawBufferIndexedIndirect(NULL,
															 buffers.viewModel.indices,
															 0,
															 VK_INDEX_TYPE_UINT32,
															 pipelines.viewModel,
															 &pipelineBindInfo,
															 buffers.viewModel.drawInfo,
															 0,
															 buffers.viewModel.drawCount,
															 sizeof(VkDrawIndexedIndirectCommand)),
							   "Failed to draw view model!");
	}

	if (UnlockLodThreadMutex() != 0)
	{
		LogError("Failed to unlock LOD thread mutex with error: %s", SDL_GetError());
		return VK_ERROR_UNKNOWN;
	}

	return VK_SUCCESS;
}

bool VK_Cleanup()
{
	VulkanTest(lunaDestroyInstance(), "Cleanup failed!");
	VulkanActorsVariablesCleanup();
	free(buffers.ui.vertices.data);
	free(buffers.ui.indices.data);
	free(buffers.viewModel.instanceDatas);
	free(buffers.sky.vertices.data);
	free(buffers.sky.indices.data);
	free(buffers.walls.vertices.data);
	free(buffers.walls.indices.data);
	free(buffers.actorWalls.vertices.data);
	free(buffers.actorWalls.indices.data);
	free(buffers.actorWalls.instanceData.data);
	free(buffers.actorWalls.drawInfo.data);
	free(buffers.actorModels.vertices.data);
	free(buffers.actorModels.indices.data);
	free(buffers.actorModels.instanceData.data);
	free(buffers.actorModels.shadedDrawInfo.data);
	free(buffers.actorModels.unshadedDrawInfo.data);

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

bool VK_LoadLevelWalls(const Level *level)
{
	if (!level->hasCeiling)
	{
		VulkanTest(LoadSky(LoadModel(MODEL("model_sky"))), "Failed to load sky!");
	}

	pushConstants.roofTextureIndex = TextureIndex(level->ceilOrSkyTex);
	pushConstants.floorTextureIndex = TextureIndex(level->floorTex);

	pushConstants.fogStart = (float)level->fogStart;
	pushConstants.fogEnd = (float)level->fogEnd;
	pushConstants.fogColor = level->fogColor;

	buffers.walls.objectCount = level->walls.length;
	buffers.walls.vertices.bytesUsed = sizeof(WallVertex) * 4 * buffers.walls.objectCount;
	buffers.walls.indices.bytesUsed = sizeof(uint32_t) * 6 * buffers.walls.objectCount;
	VulkanTest(ResizeWallBuffers(), "Failed to resize wall buffers!");
	LoadWalls(level);

	if (LockLodThreadMutex() != 0)
	{
		LogError("Failed to lock LOD thread mutex with error: %s", SDL_GetError());
		return false;
	}
	loadedLevel = level;
	if (!VK_UpdateActors(&level->actors, true))
	{
		VulkanLogError("Failed to load actors!");
	}
	if (UnlockLodThreadMutex() != 0)
	{
		LogError("Failed to unlock LOD thread mutex with error: %s", SDL_GetError());
		return false;
	}
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

void VK_SetTexParams(const char *texture, const bool linear, const bool repeat)
{
	const uint32_t textureIndex = TextureIndex(texture);
	LunaDescriptorImageInfo imageInfo = {
		.image = ListGetPointer(textures, textureIndex),
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
	}
	lunaWriteDescriptorSets(MAX_FRAMES_IN_FLIGHT, writeDescriptors);
}
