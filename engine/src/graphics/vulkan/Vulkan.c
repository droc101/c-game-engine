//
// Created by Noah on 7/5/2024.
//

#include <assert.h>
#include <cglm/types.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/vulkan/Vulkan.h>
#include <engine/graphics/vulkan/VulkanActors.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanInternal.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/Map.h>
#include <engine/structs/Viewmodel.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/threads/LodThread.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaInstance.h>
#include <luna/lunaTypes.h>
#include <math.h>
#include <SDL_error.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>
#ifdef JPH_DEBUG_RENDERER
#include <engine/debug/JoltDebugRenderer.h>
#include <engine/subsystem/Error.h>
#endif

static const Map *loadedLevel;
static uint8_t currentFrame;

bool VK_Init(SDL_Window *window)
{
	LogDebug("Initializing Vulkan renderer...\n");
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

bool VK_LoadLevelWalls(const Map *level)
{
	VulkanTest(LoadSky(LoadModel(MODEL("sky"))), "Failed to load sky!");

	// pushConstants.roofTextureIndex = TextureIndex(level->ceilOrSkyTex);
	// pushConstants.floorTextureIndex = TextureIndex(level->floorTex);

	pushConstants.fogStart = (float)level->fogStart;
	pushConstants.fogEnd = (float)level->fogEnd;
	pushConstants.fogColor = COLOR(level->fogColor);

	// buffers.walls.vertices.bytesUsed = sizeof(WallVertex) * 4 * level->walls.length;
	// buffers.walls.indices.bytesUsed = sizeof(uint32_t) * 6 * level->walls.length;
	// VulkanTest(ResizeWallBuffers(), "Failed to resize wall buffers!");
	// LoadWalls(level);

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

	buffers.ui.vertices.bytesUsed = 0;
	buffers.ui.indices.bytesUsed = 0;
#ifdef JPH_DEBUG_RENDERER
	buffers.debugDrawLines.vertexCount = 0;
	buffers.debugDrawLines.vertices.bytesUsed = 0;
	buffers.debugDrawTriangles.vertexCount = 0;
	buffers.debugDrawTriangles.vertices.bytesUsed = 0;
#endif

	return VK_SUCCESS;
}

VkResult VK_RenderLevel(const Map *level, const Camera *camera, const Viewmodel *viewmodel)
{
	if (loadedLevel != level)
	{
		if (!VK_LoadLevelWalls(level))
		{
			return VK_ERROR_UNKNOWN;
		}
	}
	pushConstants.cameraPosition.x = (float)camera->transform.position.x;
	pushConstants.cameraPosition.y = (float)camera->transform.position.y;
	pushConstants.cameraPosition.z = (float)camera->transform.position.z;
	pushConstants.yaw = JPH_Quat_GetRotationAngle(&camera->transform.rotation, &Vector3_AxisY) + 1.5f * PIf;
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
			.bindInfo.viewportBindInfo = &viewportBindInfo,
		},
		{
			.dynamicStateType = VK_DYNAMIC_STATE_SCISSOR,
			.bindInfo.scissorBindInfo = &scissorBindInfo,
		},
	};
	const LunaGraphicsPipelineBindInfo pipelineBindInfo = {
		.descriptorSetBindInfo.descriptorSetCount = 1,
		.descriptorSetBindInfo.descriptorSets = &descriptorSets[currentFrame],
		.dynamicStateCount = sizeof(dynamicStateBindInfos) / sizeof(*dynamicStateBindInfos),
		.dynamicStates = dynamicStateBindInfos,
	};

#ifdef JPH_DEBUG_RENDERER
	if (buffers.debugDrawLines.shouldResize || buffers.debugDrawTriangles.shouldResize)
	{
		VulkanTestReturnResult(ResizeDebugDrawBuffers(), "Failed to resize debug draw buffer!");
	}
	if (true) // not has ceiling
	{
		VulkanTestReturnResult(lunaDrawBufferIndexed(buffers.sky.vertices.buffer,
													 buffers.sky.indices.buffer,
													 VK_INDEX_TYPE_UINT32,
													 pipelines.sky,
													 &pipelineBindInfo,
													 buffers.sky.indexCount,
													 1,
													 0,
													 0,
													 0),
							   "Failed to draw sky!");
#ifdef JPH_DEBUG_RENDERER_WIREFRAME
		VulkanTestReturnResult(lunaDrawBuffer(NULL, pipelines.floorAndCeiling, &pipelineBindInfo, 6, 1, 0, 0),
							   "Failed to draw floor!");
	}

	if (false) // has ceiling
	{
		VulkanTestReturnResult(lunaDrawBuffer(NULL, pipelines.floorAndCeiling, &pipelineBindInfo, 12, 1, 0, 0),
							   "Failed to draw floor and ceiling!");
	}
	if (buffers.walls.indices.bytesUsed)
	{
		VulkanTestReturnResult(lunaDrawBufferIndexed(buffers.walls.vertices.buffer,
													 buffers.walls.indices.buffer,
													 VK_INDEX_TYPE_UINT32,
													 pipelines.walls,
													 &pipelineBindInfo,
													 buffers.walls.indices.bytesUsed / sizeof(uint32_t),
													 1,
													 0,
													 0,
													 0),
							   "Failed to draw walls!");
	}
#else
	}
#endif
	if (buffers.debugDrawLines.vertexCount)
	{
		lunaWriteDataToBuffer(buffers.debugDrawLines.vertices.buffer,
							  buffers.debugDrawLines.vertices.data,
							  buffers.debugDrawLines.vertices.bytesUsed,
							  0);
		VulkanTestReturnResult(lunaDrawBuffer(buffers.debugDrawLines.vertices.buffer,
											  pipelines.debugDrawLines,
											  &pipelineBindInfo,
											  buffers.debugDrawLines.vertexCount,
											  1,
											  0,
											  0),
							   "Failed to draw Jolt debug renderer lines!");
	}
	if (buffers.debugDrawTriangles.vertexCount)
	{
		lunaWriteDataToBuffer(buffers.debugDrawTriangles.vertices.buffer,
							  buffers.debugDrawTriangles.vertices.data,
							  buffers.debugDrawTriangles.vertices.bytesUsed,
							  0);
		VulkanTestReturnResult(lunaDrawBuffer(buffers.debugDrawTriangles.vertices.buffer,
											  pipelines.debugDrawTriangles,
											  &pipelineBindInfo,
											  buffers.debugDrawTriangles.vertexCount,
											  1,
											  0,
											  0),
							   "Failed to draw Jolt debug renderer triangles!");
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
#else
	VulkanTestReturnResult(lunaDrawBufferIndexed(buffers.sky.vertices.buffer,
												 buffers.sky.indices.buffer,
												 VK_INDEX_TYPE_UINT32,
												 pipelines.sky,
												 &pipelineBindInfo,
												 buffers.sky.indexCount,
												 1,
												 0,
												 0,
												 0),
						   "Failed to draw sky!");
	VulkanTestReturnResult(lunaDrawBuffer(NULL, pipelines.floorAndCeiling, &pipelineBindInfo, 6, 1, 0, 0),
						   "Failed to draw floor!");

	if (buffers.walls.indices.bytesUsed)
	{
		VulkanTestReturnResult(lunaDrawBufferIndexed(buffers.walls.vertices.buffer,
													 buffers.walls.indices.buffer,
													 VK_INDEX_TYPE_UINT32,
													 pipelines.walls,
													 &pipelineBindInfo,
													 buffers.walls.indices.bytesUsed / sizeof(uint32_t),
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
															 VK_INDEX_TYPE_UINT32,
															 pipelines.viewModel,
															 &pipelineBindInfo,
															 buffers.viewModel.drawInfo,
															 0,
															 buffers.viewModel.drawCount,
															 sizeof(VkDrawIndexedIndirectCommand)),
							   "Failed to draw view model!");
	}
#endif

	if (UnlockLodThreadMutex() != 0)
	{
		LogError("Failed to unlock LOD thread mutex with error: %s", SDL_GetError());
		return VK_ERROR_UNKNOWN;
	}

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

		buffers.ui.shouldResize = false;
	}
	if (buffers.ui.indices.bytesUsed > 0)
	{
		lunaWriteDataToBuffer(buffers.ui.vertices.buffer, buffers.ui.vertices.data, buffers.ui.vertices.bytesUsed, 0);
		lunaWriteDataToBuffer(buffers.ui.indices.buffer, buffers.ui.indices.data, buffers.ui.indices.bytesUsed, 0);
	}

	if (LockLodThreadMutex() != 0)
	{
		LogError("Failed to lock LOD thread mutex with error: %s", SDL_GetError());
		return VK_ERROR_UNKNOWN;
	}

	if (buffers.ui.indices.bytesUsed > 0)
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
				.bindInfo.viewportBindInfo = &viewportBindInfo,
			},
			{
				.dynamicStateType = VK_DYNAMIC_STATE_SCISSOR,
				.bindInfo.scissorBindInfo = &scissorBindInfo,
			},
		};
		const LunaGraphicsPipelineBindInfo pipelineBindInfo = {
			.descriptorSetBindInfo.descriptorSetCount = 1,
			.descriptorSetBindInfo.descriptorSets = &descriptorSets[currentFrame],
			.dynamicStateCount = sizeof(dynamicStateBindInfos) / sizeof(*dynamicStateBindInfos),
			.dynamicStates = dynamicStateBindInfos,
		};
		VulkanTestReturnResult(lunaDrawBufferIndexed(buffers.ui.vertices.buffer,
													 buffers.ui.indices.buffer,
													 VK_INDEX_TYPE_UINT32,
													 pipelines.ui,
													 &pipelineBindInfo,
													 buffers.ui.indices.bytesUsed / sizeof(uint32_t),
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

bool VK_Cleanup()
{
	LogDebug("Cleaning up Vulkan renderer...\n");
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

void VK_DrawColoredQuad(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const Color color)
{
	DrawRectInternal(VK_X_TO_NDC(x), VK_Y_TO_NDC(y), VK_X_TO_NDC(x + w), VK_Y_TO_NDC(y + h), 0, 0, 0, 0, &color, -1);
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
		DrawQuadInternal(matrix, &color, -1);
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
					 &COLOR_WHITE,
					 TextureIndex(texture));
}

void VK_DrawTexturedQuadMod(const int32_t x,
							const int32_t y,
							const int32_t w,
							const int32_t h,
							const char *texture,
							const Color *color)
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
					 &COLOR_WHITE,
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
					 &color,
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
		DrawQuadInternal(matrix, &color, TextureIndex(texture));
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
	DrawQuadInternal(matrix, &color, -1);
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

void VK_DrawUiTriangles(const UiTriangleArray *triangleArray, const char *texture, const Color color)
{
	EnsureSpaceForUiElements(triangleArray->vertexCount, triangleArray->indexCount);

	UiVertex *vertices = buffers.ui.vertices.data + buffers.ui.vertices.bytesUsed;
	uint32_t *indices = buffers.ui.indices.data + buffers.ui.indices.bytesUsed;
	const uint32_t vertexOffset = buffers.ui.vertices.bytesUsed / sizeof(UiVertex);

	for (size_t i = 0; i < triangleArray->vertexCount; i++)
	{
		memcpy(vertices + i, triangleArray->vertices[i], sizeof(*triangleArray->vertices));
		vertices[i].r = color.r;
		vertices[i].g = color.g;
		vertices[i].b = color.b;
		vertices[i].a = color.a;
		vertices[i].textureIndex = TextureIndex(texture);
	}
	for (size_t i = 0; i < triangleArray->indexCount; i++)
	{
		indices[i] = (*triangleArray->indices)[i] + vertexOffset;
	}

	buffers.ui.vertices.bytesUsed += triangleArray->vertexCount * sizeof(UiVertex);
	buffers.ui.indices.bytesUsed += triangleArray->indexCount * sizeof(uint32_t);
}

void VK_DrawJoltDebugRendererLine(const Vector3 *from, const Vector3 *to, const uint32_t color)
{
#ifdef JPH_DEBUG_RENDERER
	if (buffers.debugDrawLines.vertices.allocatedSize <
		buffers.debugDrawLines.vertices.bytesUsed + sizeof(DebugDrawVertex) * 2)
	{
		buffers.debugDrawLines.vertices.allocatedSize += sizeof(DebugDrawVertex) * 2 * 16;
		buffers.debugDrawLines.shouldResize = true;

		DebugDrawVertex *newVertices = realloc(buffers.debugDrawLines.vertices.data,
											   buffers.debugDrawLines.vertices.allocatedSize);
		CheckAlloc(newVertices);
		buffers.debugDrawLines.vertices.data = newVertices;
	}

	const float a = 1;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	DebugDrawVertex *bufferVertices = buffers.debugDrawLines.vertices.data + buffers.debugDrawLines.vertices.bytesUsed;

	bufferVertices[0] = (DebugDrawVertex){
		.position = *from,
		.color = {r, g, b, a},
	};
	bufferVertices[1] = (DebugDrawVertex){
		.position = *to,
		.color = {r, g, b, a},
	};

	buffers.debugDrawLines.vertexCount += 2;
	buffers.debugDrawLines.vertices.bytesUsed += sizeof(DebugDrawVertex) * 2;
#else
	(void)from;
	(void)to;
	(void)color;
#endif
}

void VK_DrawJoltDebugRendererTriangle(const Vector3 *vertices, const uint32_t color)
{
#ifdef JPH_DEBUG_RENDERER
	if (buffers.debugDrawTriangles.vertices.allocatedSize <
		buffers.debugDrawTriangles.vertices.bytesUsed + sizeof(DebugDrawVertex) * 3)
	{
		buffers.debugDrawTriangles.vertices.allocatedSize += sizeof(DebugDrawVertex) * 3 * 16;
		buffers.debugDrawTriangles.shouldResize = true;

		DebugDrawVertex *newVertices = realloc(buffers.debugDrawTriangles.vertices.data,
											   buffers.debugDrawTriangles.vertices.allocatedSize);
		CheckAlloc(newVertices);
		buffers.debugDrawTriangles.vertices.data = newVertices;
	}

	const float a = 1;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	DebugDrawVertex *bufferVertices = buffers.debugDrawTriangles.vertices.data +
									  buffers.debugDrawTriangles.vertices.bytesUsed;

	bufferVertices[0] = (DebugDrawVertex){
		.position = vertices[0],
		.color = {r, g, b, a},
	};
	bufferVertices[1] = (DebugDrawVertex){
		.position = vertices[1],
		.color = {r, g, b, a},
	};
	bufferVertices[2] = (DebugDrawVertex){
		.position = vertices[2],
		.color = {r, g, b, a},
	};

	buffers.debugDrawTriangles.vertexCount += 3;
	buffers.debugDrawTriangles.vertices.bytesUsed += sizeof(DebugDrawVertex) * 3;
#else
	(void)vertices;
	(void)color;
#endif
}
