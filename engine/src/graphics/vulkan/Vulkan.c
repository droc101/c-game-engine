//
// Created by Noah on 7/5/2024.
//

#include <assert.h>
#include <cglm/types.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/graphics/vulkan/Vulkan.h>
#include <engine/graphics/vulkan/VulkanActors.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanInternal.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/Map.h>
#include <engine/structs/Vector2.h>
#include <engine/structs/Viewmodel.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/threads/LodThread.h>
#include <joltc/Math/Vector3.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaInstance.h>
#include <luna/lunaTypes.h>
#include <math.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "luna/luna.h"

#ifdef JPH_DEBUG_RENDERER
#include <engine/debug/JoltDebugRenderer.h>
#include <engine/subsystem/Error.h>
#endif

// TODO: Can the concept of frames in flight be removed entirely in favor of simply letting Luna handle it?
//  I've started this process, so if it cannot it needs to be readded in several places

static const Map *loadedMap;
static size_t skyModelIndexCount;

static inline VkResult LoadSky(const ModelDefinition *model)
{
	if (model->skinCount > 1)
	{
		LogWarning("Discarding %d extra skins from sky model!\n", model->skinCount - 1);
	}
	if (model->materialCount > 1)
	{
		LogWarning("Discarding %d extra materials from sky model!\n", model->materialCount - 1);
	}
	if (model->materials->shader != SHADER_SKY)
	{
		LogWarning("Ignoring incorrect material shader type on sky model!\n");
	}
	if (model->lodCount > 1)
	{
		LogWarning("Discarding %d extra lods from sky model!\n", model->lodCount - 1);
	}

	const ModelLod *lod = model->lods;

	SkyVertex vertices[lod->vertexCount];
	for (size_t i = 0; i < lod->vertexCount; i++)
	{
		memcpy(vertices + i, lod->vertexData + i, sizeof(SkyVertex));
	}
	assert(lunaGetBufferSize(buffers.sky.vertices) == sizeof(SkyVertex) * lod->vertexCount);
	const LunaBufferWriteInfo vertexBufferWriteInfo = {
		.bytes = sizeof(SkyVertex) * lod->vertexCount,
		.data = vertices,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.sky.vertices, &vertexBufferWriteInfo),
						   "Failed to write sky model vertex data to buffer!");

	skyModelIndexCount = lod->indexCount[0];
	assert(lunaGetBufferSize(buffers.sky.indices) == sizeof(uint32_t) * skyModelIndexCount);
	const LunaBufferWriteInfo indexBufferWriteInfo = {
		.bytes = sizeof(uint32_t) * skyModelIndexCount,
		.data = lod->indexData[0],
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.sky.indices, &indexBufferWriteInfo),
						   "Failed to write sky model index data to buffer!");

	return VK_SUCCESS;
}

static inline VkResult UpdateViewmodel(const Viewmodel *viewmodel)
{
	const ModelDefinition *model = viewmodel->model;
	const uint32_t *materialIndices = model->skinMaterialIndices[viewmodel->modelSkin];
	uint32_t shadedCount = 0;
	uint32_t indexCount = 0;
	for (uint32_t i = 0; i < model->materialSlotCount; i++)
	{
		const Material *material = model->materials + materialIndices[i];

		ModelInstanceData instanceData;
		instanceData.materialColor = material->color;
		instanceData.textureIndex = TextureIndex(material->texture);
		const LunaBufferWriteInfo instanceDataBufferWriteInfo = {
			.bytes = SizeofMember(ModelInstanceData, materialColor) + SizeofMember(ModelInstanceData, textureIndex),
			.data = &(instanceData.materialColor),
			.offset = i * sizeof(ModelInstanceData) + offsetof(ModelInstanceData, materialColor),
			.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		};
		VulkanTestReturnResult(lunaWriteDataToBuffer(device,
													 commandBuffer,
													 buffers.viewmodel.instanceData,
													 &instanceDataBufferWriteInfo),
							   "Failed to write viewmodel instance data to buffer!");

		VkDrawIndexedIndirectCommand drawInfo = {
			.indexCount = model->lods->indexCount[i],
			.instanceCount = 1,
			.firstIndex = indexCount,
			.firstInstance = i,
		};
		const LunaBufferWriteInfo drawInfoBufferWriteInfo = {
			.bytes = sizeof(VkDrawIndexedIndirectCommand),
			.data = &drawInfo,
			.offset = (material->shader == SHADER_SHADED ? shadedCount : i - shadedCount) *
					  sizeof(VkDrawIndexedIndirectCommand),
			.stageFlags = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		};
		VulkanTestReturnResult(lunaWriteDataToBuffer(device,
													 commandBuffer,
													 material->shader == SHADER_SHADED
															 ? buffers.viewmodel.shadedDrawInfo
															 : buffers.viewmodel.unshadedDrawInfo,
													 &drawInfoBufferWriteInfo),
							   "Failed to write viewmodel draw info to buffer!");

		shadedCount += material->shader == SHADER_SHADED ? 1 : 0;
		indexCount += model->lods->indexCount[i];
	}

	return VK_SUCCESS;
}

static inline VkResult LoadViewmodel(const Viewmodel *viewmodel)
{
	const ModelDefinition *model = viewmodel->model;
	const uint32_t *materialIndices = model->skinMaterialIndices[viewmodel->modelSkin];
	const ModelLod *lod = model->lods;

	uint32_t indices[lod->totalIndexCount];
	size_t indexCount = 0;
	size_t shadedMaterialCount = 0;
	size_t unshadedMaterialCount = 0;
	for (size_t i = 0; i < model->materialSlotCount; i++)
	{
		memcpy(indices + indexCount, lod->indexData[i], lod->indexCount[i] * sizeof(uint32_t));
		indexCount += lod->indexCount[i];
		if (model->materials[materialIndices[i]].shader == SHADER_SHADED)
		{
			shadedMaterialCount++;
		} else
		{
			assert(model->materials[materialIndices[i]].shader == SHADER_UNSHADED);
			unshadedMaterialCount++;
		}
	}

	const size_t vertexBufferSize = lod->vertexCount * sizeof(ModelVertex);
	VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.viewmodel.vertices, vertexBufferSize),
						   "Failed to resize viewmodel vertex buffer!");
	const size_t indexBufferSize = lod->totalIndexCount * sizeof(uint32_t);
	VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.viewmodel.indices, indexBufferSize),
						   "Failed to resize viewmodel index buffer!");
	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffers.viewmodel.instanceData,
											(shadedMaterialCount + unshadedMaterialCount) * sizeof(ModelInstanceData)),
						   "Failed to resize viewmodel instance data buffer!");
	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffers.viewmodel.shadedDrawInfo,
											shadedMaterialCount * sizeof(VkDrawIndexedIndirectCommand)),
						   "Failed to resize viewmodel shaded material draw info buffer!");
	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffers.viewmodel.unshadedDrawInfo,
											unshadedMaterialCount * sizeof(VkDrawIndexedIndirectCommand)),
						   "Failed to resize viewmodel unshaded material draw info buffer!");

	const LunaBufferWriteInfo vertexBufferWriteInfo = {
		.bytes = vertexBufferSize,
		.data = lod->vertexData,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.viewmodel.vertices,
												 &vertexBufferWriteInfo),
						   "Failed to write data to viewmodel vertex buffer!");
	const LunaBufferWriteInfo indexBufferWriteInfo = {
		.bytes = indexBufferSize,
		.data = indices,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.viewmodel.indices,
												 &indexBufferWriteInfo),
						   "Failed to write data to viewmodel index buffer!");


	VulkanTestReturnResult(UpdateViewmodel(viewmodel), "Failed to update viewmodel!");

	return VK_SUCCESS;
}

static inline VkResult LoadMapModelsToBuffer(const size_t modelCount, const MapModel *models)
{
	size_t totalVertexCount = 0;
	size_t totalIndexCount = 0;
	size_t totalMaterialCount = 0;
	size_t shadedMaterialCount = 0;
	size_t unshadedMaterialCount = 0;
	for (size_t i = 0; i < modelCount; i++)
	{
		const MapModel *model = models + i;
		totalVertexCount += model->vertexCount;
		totalIndexCount += model->indexCount;
		totalMaterialCount++;
		const ModelShader shader = model->material->shader;
		switch (shader)
		{
			case SHADER_SHADED:
				shadedMaterialCount++;
				break;
			case SHADER_UNSHADED:
				unshadedMaterialCount++;
				break;
			default:
				return VK_ERROR_UNKNOWN;
		}
	}
	const size_t vertexBufferSize = totalVertexCount * sizeof(MapVertex);
	VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.map.vertices, vertexBufferSize),
						   "Failed to resize map vertex buffer!");
	const size_t indexBufferSize = totalIndexCount * sizeof(uint32_t);
	VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.map.indices, indexBufferSize),
						   "Failed to resize map index buffer!");
	const size_t instanceDataBufferSize = totalMaterialCount * sizeof(uint32_t);
	VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.map.instanceData, instanceDataBufferSize),
						   "Failed to resize map instance data buffer!");
	const size_t shadedDrawInfoBufferSize = shadedMaterialCount * sizeof(VkDrawIndexedIndirectCommand);
	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffers.map.shadedDrawInfo,
											shadedDrawInfoBufferSize),
						   "Failed to resize map shaded draw info buffer!");
	const size_t unshadedDrawInfoBufferSize = unshadedMaterialCount * sizeof(VkDrawIndexedIndirectCommand);
	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffers.map.unshadedDrawInfo,
											unshadedDrawInfoBufferSize),
						   "Failed to resize map unshaded draw info buffer!");

	VkDeviceSize vertexOffset = 0;
	VkDeviceSize indexOffset = 0;
	size_t shadedMaterialIndex = 0;
	size_t unshadedMaterialIndex = 0;
	MapVertex vertices[totalVertexCount];
	uint32_t indices[totalIndexCount];
	uint32_t textureIndices[totalMaterialCount];
	VkDrawIndexedIndirectCommand shadedDrawInfo[shadedMaterialCount];
	VkDrawIndexedIndirectCommand unshadedDrawInfo[unshadedMaterialCount];
	for (size_t i = 0; i < modelCount; i++)
	{
		const MapModel *model = models + i;
		memcpy(vertices + vertexOffset, model->vertices, model->vertexCount * sizeof(MapVertex));
		memcpy(indices + indexOffset, model->indices, model->indexCount * sizeof(uint32_t));
		textureIndices[i] = TextureIndex(model->material->texture);
		switch (models[i].material->shader)
		{
			case SHADER_SHADED:
				shadedDrawInfo[shadedMaterialIndex].indexCount = model->indexCount;
				shadedDrawInfo[shadedMaterialIndex].instanceCount = 1;
				shadedDrawInfo[shadedMaterialIndex].firstIndex = indexOffset;
				shadedDrawInfo[shadedMaterialIndex].vertexOffset = (int32_t)vertexOffset;
				shadedDrawInfo[shadedMaterialIndex].firstInstance = i;
				shadedMaterialIndex++;
				break;
			case SHADER_UNSHADED:
				unshadedDrawInfo[unshadedMaterialIndex].indexCount = model->indexCount;
				unshadedDrawInfo[unshadedMaterialIndex].instanceCount = 1;
				unshadedDrawInfo[unshadedMaterialIndex].firstIndex = indexOffset;
				unshadedDrawInfo[unshadedMaterialIndex].vertexOffset = (int32_t)vertexOffset;
				unshadedDrawInfo[unshadedMaterialIndex].firstInstance = i;
				unshadedMaterialIndex++;
				break;
			default:
				// Impossible to hit
				return VK_ERROR_UNKNOWN;
		}

		vertexOffset += model->vertexCount;
		indexOffset += model->indexCount;
	}

	const LunaBufferWriteInfo vertexBufferWriteInfo = {
		.bytes = vertexBufferSize,
		.data = vertices,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.map.vertices, &vertexBufferWriteInfo),
						   "Failed to write data to map vertex buffer!");
	const LunaBufferWriteInfo indexBufferWriteInfo = {
		.bytes = indexBufferSize,
		.data = indices,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.map.indices, &indexBufferWriteInfo),
						   "Failed to write data to map index buffer!");
	const LunaBufferWriteInfo instanceDataBufferWriteInfo = {
		.bytes = instanceDataBufferSize,
		.data = textureIndices,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.map.instanceData,
												 &instanceDataBufferWriteInfo),
						   "Failed to write data to map per-material data buffer!");
	const LunaBufferWriteInfo shadedDrawInfoBufferWriteInfo = {
		.bytes = shadedDrawInfoBufferSize,
		.data = shadedDrawInfo,
		.stageFlags = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.map.shadedDrawInfo,
												 &shadedDrawInfoBufferWriteInfo),
						   "Failed to write data to map shaded draw info buffer!");
	const LunaBufferWriteInfo unshadedDrawInfoBufferWriteInfo = {
		.bytes = unshadedDrawInfoBufferSize,
		.data = unshadedDrawInfo,
		.stageFlags = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.map.unshadedDrawInfo,
												 &unshadedDrawInfoBufferWriteInfo),
						   "Failed to write data to map unshaded draw info buffer!");

	return VK_SUCCESS;
}

static inline VkResult DrawSky(const LunaGraphicsPipelineBindInfo *pipelineBindInfo)
{
	if (skyModelIndexCount == 0)
	{
		return VK_SUCCESS;
	}

	VulkanTestReturnResult(lunaPushConstants(device, commandBuffer, pipelines.sky),
						   "Failed to push constants for sky pipeline!");
	const LunaDrawIndexedInfo skyDrawInfo = {
		.pipeline = pipelines.sky,
		.pipelineBindInfo = pipelineBindInfo,
		.indexCount = skyModelIndexCount,
		.instanceCount = 1,
	};
	VulkanTestReturnResult(lunaDrawBufferIndexed(device,
												 commandBuffer,
												 buffers.sky.vertices,
												 buffers.sky.indices,
												 VK_INDEX_TYPE_UINT32,
												 &skyDrawInfo),
						   "Failed to draw sky!");

	return VK_SUCCESS;
}

static inline VkResult DrawMap(const LunaGraphicsPipelineBindInfo *pipelineBindInfo)
{
	const size_t shadedDrawCount = lunaGetBufferSize(buffers.map.shadedDrawInfo) / sizeof(VkDrawIndexedIndirectCommand);
	const size_t unshadedDrawCount = lunaGetBufferSize(buffers.map.unshadedDrawInfo) /
									 sizeof(VkDrawIndexedIndirectCommand);

	if (shadedDrawCount != 0 || unshadedDrawCount != 0)
	{
		VulkanTest(lunaBindVertexBuffers(device,
										 commandBuffer,
										 (LunaBuffer[]){buffers.map.vertices, buffers.map.instanceData},
										 0,
										 2),
				   "Failed to bind map vertex buffers!");
		VulkanTest(lunaBindIndexBuffer(device, commandBuffer, buffers.map.indices, VK_INDEX_TYPE_UINT32),
				   "Failed to bind map index buffer!");
	}

	if (shadedDrawCount != 0)
	{
		const LunaDrawIndexedIndirectInfo drawInfo = {
			.pipeline = pipelines.shadedMap,
			.pipelineBindInfo = pipelineBindInfo,
			.buffer = buffers.map.shadedDrawInfo,
			.drawCount = shadedDrawCount,
		};
		VulkanTestReturnResult(lunaDrawIndexedIndirect(device, commandBuffer, &drawInfo), "Failed to draw shaded map!");
	}

	if (unshadedDrawCount != 0)
	{
		const LunaDrawIndexedIndirectInfo drawInfo = {
			.pipeline = pipelines.unshadedMap,
			.pipelineBindInfo = pipelineBindInfo,
			.buffer = buffers.map.unshadedDrawInfo,
			.drawCount = unshadedDrawCount,
		};
		VulkanTestReturnResult(lunaDrawIndexedIndirect(device, commandBuffer, &drawInfo),
							   "Failed to draw unshaded map!");
	}

	return VK_SUCCESS;
}

static inline VkResult DrawViewmodel(const LunaGraphicsPipelineBindInfo *pipelineBindInfo)
{
	const size_t shadedDrawCount = lunaGetBufferSize(buffers.viewmodel.shadedDrawInfo) /
								   sizeof(VkDrawIndexedIndirectCommand);
	const size_t unshadedDrawCount = lunaGetBufferSize(buffers.viewmodel.unshadedDrawInfo) /
									 sizeof(VkDrawIndexedIndirectCommand);

	if (shadedDrawCount != 0 || unshadedDrawCount != 0)
	{
		VulkanTest(lunaBindVertexBuffers(device,
										 commandBuffer,
										 (LunaBuffer[]){buffers.viewmodel.vertices, buffers.viewmodel.instanceData},
										 0,
										 2),
				   "Failed to bind viewmodel vertex buffers!");
		VulkanTest(lunaBindIndexBuffer(device, commandBuffer, buffers.viewmodel.indices, VK_INDEX_TYPE_UINT32),
				   "Failed to bind viewmodel index buffer!");
	}

	if (shadedDrawCount != 0)
	{
		const LunaDrawIndexedIndirectInfo drawInfo = {
			.pipeline = pipelines.shadedViewmodel,
			.pipelineBindInfo = pipelineBindInfo,
			.buffer = buffers.viewmodel.shadedDrawInfo,
			.drawCount = shadedDrawCount,
		};
		VulkanTestReturnResult(lunaDrawIndexedIndirect(device, commandBuffer, &drawInfo),
							   "Failed to draw shaded viewmodel!");
	}

	if (unshadedDrawCount != 0)
	{
		const LunaDrawIndexedIndirectInfo drawInfo = {
			.pipeline = pipelines.unshadedViewmodel,
			.pipelineBindInfo = pipelineBindInfo,
			.buffer = buffers.viewmodel.unshadedDrawInfo,
			.drawCount = unshadedDrawCount,
		};
		VulkanTestReturnResult(lunaDrawIndexedIndirect(device, commandBuffer, &drawInfo),
							   "Failed to draw unshaded viewmodel!");
	}

	return VK_SUCCESS;
}

static inline bool HandleRendererQueuedActions()
{
	const RendererQueuedAction handledActionTypes = QUEUED_ACTION_CLEAR_ALL_TEXTURES | QUEUED_ACTION_CLEAR_ALL_MODELS;
	if (rendererQueuedActions & QUEUED_ACTION_CLEAR_ALL_TEXTURES)
	{
		if (!ClearTextureCache())
		{
			return false;
		}
	}
	if (rendererQueuedActions & QUEUED_ACTION_CLEAR_ALL_MODELS)
	{
		ClearModelCache();
	}
	if ((rendererQueuedActions & handledActionTypes) != 0)
	{
		if (!VK_LoadMap(loadedMap))
		{
			return false;
		}
	}
	rendererQueuedActions &= ~handledActionTypes;

	return true;
}

bool VK_PreInit()
{
	LogDebug("Creating Vulkan instance...\n");
	if (!CreateInstance())
	{
		VK_Cleanup();

		return false;
	}

	return true;
}

bool VK_Init(SDL_Window *window)
{
	LogDebug("Initializing Vulkan renderer...\n");
	// clang-format off
	if (CreateSurface(window) && CreateLogicalDevice() && CreateCommandBuffers() && CreateSwapchain() &&
		CreateRenderPass() && CreateDescriptorSetLayouts() && CreateGraphicsPipelines() && CreateTextureSamplers() &&
		CreateBuffers() && CreateDescriptorSet())
	{
		// clang-format on

		VkPhysicalDeviceProperties physicalDeviceProperties;
		lunaGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
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

		InitActorLoadingVariables();

		return true;
	}

	VK_Cleanup();

	return false;
}

bool VK_FrameStart()
{
	if (minimized)
	{
		return false;
	}

	if (!HandleRendererQueuedActions())
	{
		return false;
	}

	VulkanTestResizeSwapchain(lunaBeginFrame(device, commandBuffer, false), "Failed to begin frame!");
	const LunaRenderPassBeginInfo beginInfo = {
		.renderArea.extent = swapChainExtent,
		.depthAttachmentClearValue.depthStencil.depth = 1,
	};
	VulkanTestResizeSwapchain(lunaBeginRenderPass(device, commandBuffer, renderPass, &beginInfo),
							  "Failed to begin render pass!");

	buffers.ui.freeQuads = buffers.ui.allocatedQuads;
#ifdef JPH_DEBUG_RENDERER
	buffers.debugDrawLines.vertexCount = 0;
	buffers.debugDrawLines.vertices.bytesUsed = 0;
	buffers.debugDrawTriangles.vertexCount = 0;
	buffers.debugDrawTriangles.vertices.bytesUsed = 0;
#endif

	return true;
}

bool VK_RenderMap(const Map *map, const Camera *camera)
{
	if (map != loadedMap)
	{
		VulkanTest(VK_LoadMap(map), "Failed to load map!");
	}

	float lighting[4]; // r, g, b, a
	lighting[0] = map->lightColor.r;
	lighting[1] = map->lightColor.g;
	lighting[2] = map->lightColor.b;
	lighting[3] = map->lightColor.a;
	const LunaBufferWriteInfo lightingBufferWriteInfo = {
		.bytes = sizeof(lighting),
		.data = lighting,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
	};
	VulkanTest(lunaWriteDataToBuffer(device, commandBuffer, buffers.uniforms.lighting, &lightingBufferWriteInfo),
			   "Failed to update lighting data!");

	float fog[6]; // r, g, b, a, start, end
	fog[0] = map->fogColor.r;
	fog[1] = map->fogColor.g;
	fog[2] = map->fogColor.b;
	fog[3] = map->fogColor.a;
	fog[4] = map->fogStart;
	fog[5] = map->fogEnd;
	const LunaBufferWriteInfo fogBufferWriteInfo = {
		.bytes = sizeof(fog),
		.data = fog,
		.stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	};
	VulkanTest(lunaWriteDataToBuffer(device, commandBuffer, buffers.uniforms.fog, &fogBufferWriteInfo),
			   "Failed to update fog data!");

	VulkanTest(UpdateCameraUniform(camera), "Failed to update transform matrix!");

	VulkanTest(UpdateViewModelMatrix(&map->viewmodel), "Failed to update viewmodel transform matrix!");


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
		.descriptorSetBindInfo.descriptorSets = &descriptorSet,
		.dynamicStateCount = sizeof(dynamicStateBindInfos) / sizeof(*dynamicStateBindInfos),
		.dynamicStates = dynamicStateBindInfos,
	};

	if (map->renderSky)
	{
		VulkanTest(DrawSky(&pipelineBindInfo), "Failed to draw sky!");
	}
	VulkanTest(DrawMap(&pipelineBindInfo), "Failed to draw map!");
	if (map->viewmodel.enabled)
	{
		VulkanTest(DrawViewmodel(&pipelineBindInfo), "Failed to draw viewmodel!");
	}

	return true;
}

bool VK_FrameEnd()
{
	if ((pendingTasks & PENDING_TASK_UI_BUFFERS_RESIZE_BIT) == PENDING_TASK_UI_BUFFERS_RESIZE_BIT)
	{
		VulkanTest(lunaGrowBuffer(device,
								  commandBuffer,
								  &buffers.ui.vertexBuffer,
								  buffers.ui.allocatedQuads * 4 * sizeof(UiVertex)),
				   "Failed to recreate UI vertex buffer!");
		VulkanTest(lunaGrowBuffer(device,
								  commandBuffer,
								  &buffers.ui.indexBuffer,
								  buffers.ui.allocatedQuads * 6 * sizeof(uint32_t)),
				   "Failed to recreate UI index buffer!");

		pendingTasks &= ~PENDING_TASK_UI_BUFFERS_RESIZE_BIT;
	}
	if (buffers.ui.freeQuads != buffers.ui.allocatedQuads)
	{
		// TODO: This write is the cause of the glitching (and crash) when pausing the game
		const LunaBufferWriteInfo vertexBufferWriteInfo = {
			.bytes = (buffers.ui.allocatedQuads - buffers.ui.freeQuads) * 4 * sizeof(UiVertex),
			.data = buffers.ui.vertexData,
			.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		};
		const LunaBufferWriteInfo indexBufferWriteInfo = {
			.bytes = (buffers.ui.allocatedQuads - buffers.ui.freeQuads) * 6 * sizeof(uint32_t),
			.data = buffers.ui.indexData,
			.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		};
		VulkanTest(lunaWriteDataToBuffer(device, commandBuffer, buffers.ui.vertexBuffer, &vertexBufferWriteInfo),
				   "Failed to write UI vertex buffer!");
		VulkanTest(lunaWriteDataToBuffer(device, commandBuffer, buffers.ui.indexBuffer, &indexBufferWriteInfo),
				   "Failed to write UI index buffer!");
	}

	if (buffers.ui.freeQuads != buffers.ui.allocatedQuads)
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
			.descriptorSetBindInfo.descriptorSets = &descriptorSet,
			.dynamicStateCount = sizeof(dynamicStateBindInfos) / sizeof(*dynamicStateBindInfos),
			.dynamicStates = dynamicStateBindInfos,
		};
		const LunaDrawIndexedInfo drawInfo = {
			.pipeline = pipelines.ui,
			.pipelineBindInfo = &pipelineBindInfo,
			.indexCount = (buffers.ui.allocatedQuads - buffers.ui.freeQuads) * 6,
			.instanceCount = 1,
		};
		VulkanTest(lunaDrawBufferIndexed(device,
										 commandBuffer,
										 buffers.ui.vertexBuffer,
										 buffers.ui.indexBuffer,
										 VK_INDEX_TYPE_UINT32,
										 &drawInfo),
				   "Failed to draw UI!");
	}

	lunaEndRenderPass(commandBuffer);

	const VkSwapchainKHR swapchain = lunaGetVkSwapchain();
	uint32_t imageIndex = lunaGetSwapchainImageIndex();
	const VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &imageIndex,
	};
	const VkPipelineStageFlags2 waitStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	const LunaCommandBufferSubmitInfo submitInfo = {
		.queue = queue,
		.waitSemaphoreCount = 1,
		.waitSemaphores = &semaphore,
		.waitDstStageMasks = &waitStage,
		.signalSemaphoreCount = 1,
		.signalSemaphores = &semaphore,
	};

	VulkanTestResizeSwapchain(lunaEndFrame(device, commandBuffer, &presentInfo, &submitInfo),
							  "Failed to present swapchain!");

	return true;
}

void VK_Cleanup()
{
	LogDebug("Cleaning up Vulkan renderer...\n");
	free(buffers.ui.vertexData);
	free(buffers.ui.indexData);
	VulkanTestInternal(lunaDestroyInstance(), (void)0, "Cleanup failed!");
}

/**
 * Loads a map into VRAM. This function is responsible for
 *  1. Ensuring that the target buffers are large enough to hold the data, and resizing as needed
 *  2. Copying the data out of the Map struct and into VRAM, using temporary CPU-side buffers in order to combine
 *      all map models into one large vertex buffer and one large index buffer
 *  3. Copying any data that is only required once per material into the @c perMaterialData buffer
 *  4. Generating the @c VkDrawIndexedIndirectCommand structures that are stored in the @c drawInfo buffer
 *  5. Setting the initial state for any relevant descriptor sets or push constants
 * @todo This function should set the initial state for any descriptor sets and push constants
 * @param map The map to load
 * @return @c VK_SUCCESS if the map was successfully loaded, or a meaningful result code otherwise
 */
bool VK_LoadMap(const Map *map)
{
	if (map == NULL)
	{
		loadedMap = NULL;

		return true;
	}

	VulkanTest(LoadMapModelsToBuffer(map->modelCount, map->models), "Failed to load map models!");

	VulkanTest(LoadViewmodel(&map->viewmodel), "Failed to load viewmodel!");

	VulkanTest(LoadActors(), "Failed to load actors!");

	if (map->renderSky)
	{
		VulkanTestReturnResult(LoadSky(LoadModel(MODEL("sky"))), "Failed to load sky model!");
		skyTextureIndex = TextureIndex(map->skyTexture);
	}

	loadedMap = map;

	return true;
}

bool VK_UpdateViewportSize()
{
	const Vector2 windowSize = ActualWindowSizeIgnoreDPI();
	swapChainExtent.width = windowSize.x;
	swapChainExtent.height = windowSize.y;
	const LunaSwapchainResizeInfo swapchainResizeInfo = {
		.newSize = swapChainExtent,
		.renderPassCount = 1,
		.renderPasses = &renderPass,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTest(lunaResizeSwapchain(device, &swapchainResizeInfo), "Failed to resize swapchain!");

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
	const uint32_t textureIndex = TextureIndex(texture);
	for (int32_t i = 0; i < quadCount; i++)
	{
		DrawQuadInternal((vec4 *)(vertices + i * 16), &color, textureIndex);
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
	// Good enough for now
	const size_t quadCount = triangleArray->indexCount / 6;
	EnsureSpaceForUiElements(quadCount);

	const size_t vertexOffset = (buffers.ui.allocatedQuads - buffers.ui.freeQuads) * 4;
	UiVertex *vertices = buffers.ui.vertexData + vertexOffset;
	uint32_t *indices = buffers.ui.indexData + (buffers.ui.allocatedQuads - buffers.ui.freeQuads) * 6;

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

	buffers.ui.freeQuads -= quadCount;
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
