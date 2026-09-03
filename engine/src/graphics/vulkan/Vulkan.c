//
// Created by Noah on 7/5/2024.
//

#include <assert.h>
#include <cglm/cglm.h>
#include <cglm/clipspace/ortho_lh_zo.h>
#include <cglm/clipspace/persp_lh_zo.h>
#include <cglm/clipspace/view_lh_zo.h>
#include <cglm/types.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/debug/DPrint.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/graphics/vulkan/Vulkan.h>
#include <engine/graphics/vulkan/VulkanActors.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanInternal.h>
#include <engine/helpers/MathEx.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Light.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Options.h>
#include <engine/structs/Vector2.h>
#include <engine/structs/Viewmodel.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/joltc.h>
#include <joltc/Math/RMat44.h>
#include <joltc/Math/Vector3.h>
#include <luna/luna.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaCommandBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaImage.h>
#include <luna/lunaInstance.h>
#include <luna/lunaTypes.h>
#include <math.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>

#ifdef JPH_DEBUG_RENDERER
#include <engine/debug/JoltDebugRenderer.h>
#endif

static const Map *loadedMap;
static LunaImage lightmap = LUNA_NULL_HANDLE;
static size_t skyModelIndexCount;
static bool renderPassStarted;

static inline Map *GetMap()
{
	Map *map = GetState()->map;
	if (map != loadedMap)
	{
		if (!VK_LoadMap(map))
		{
			return false;
		}
		loadedMap = map;
	}
	return map;
}

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

	SkyVertex *vertices = malloc(sizeof(SkyVertex) * lod->vertexCount);
	CheckAlloc(vertices);
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

	free(vertices);

	skyModelIndexCount = lod->components->indexCount;
	assert(lunaGetBufferSize(buffers.sky.indices) == sizeof(uint32_t) * skyModelIndexCount);
	const LunaBufferWriteInfo indexBufferWriteInfo = {
		.bytes = sizeof(uint32_t) * skyModelIndexCount,
		.data = lod->components->indices,
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

		const uint32_t componentIndexCount = model->lods->components[i].indexCount;
		VkDrawIndexedIndirectCommand drawInfo = {
			.indexCount = componentIndexCount,
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
		indexCount += componentIndexCount;
	}

	return VK_SUCCESS;
}

static inline VkResult LoadViewmodel(const Viewmodel *viewmodel)
{
	if (viewmodel->model == NULL)
	{
		VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.viewmodel.vertices, 0),
							   "Failed to resize viewmodel vertex buffer!");
		VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.viewmodel.indices, 0),
							   "Failed to resize viewmodel index buffer!");
		VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.viewmodel.instanceData, 0),
							   "Failed to resize viewmodel instance data buffer!");
		VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.viewmodel.shadedDrawInfo, 0),
							   "Failed to resize viewmodel shaded material draw info buffer!");
		VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.viewmodel.unshadedDrawInfo, 0),
							   "Failed to resize viewmodel unshaded material draw info buffer!");
		return VK_SUCCESS;
	}
	const ModelDefinition *model = viewmodel->model;
	const uint32_t *materialIndices = model->skinMaterialIndices[viewmodel->modelSkin];
	const ModelLod *lod = model->lods;

	uint32_t indices[lod->totalIndexCount];
	size_t indexCount = 0;
	size_t shadedMaterialCount = 0;
	size_t unshadedMaterialCount = 0;
	for (size_t i = 0; i < model->materialSlotCount; i++)
	{
		const ModelComponent *component = &lod->components[i];
		memcpy(indices + indexCount, component->indices, component->indexCount * sizeof(uint32_t));
		indexCount += component->indexCount;
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

static inline VkResult CreateMapModelDrawInfos(const bool opaque,
											   MapModelsBuffer *buffer,
											   const size_t shadedDrawInfoBufferSize,
											   const size_t unshadedDrawInfoBufferSize)
{
	for (size_t i = 0; i < frustumCount; i++)
	{
		LunaBuffer *shadedDrawInfo = (LunaBuffer *)&ListGetPointer(buffer->shadedDrawInfo, i);
		LunaBuffer *unshadedDrawInfo = (LunaBuffer *)&ListGetPointer(buffer->unshadedDrawInfo, i);
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												shadedDrawInfo,
												shadedDrawInfoBufferSize + sizeof(uint32_t)),
							   "Failed to resize map shaded draw info buffer!");
		VulkanTestReturnResult(lunaResizeBuffer(device,
												commandBuffer,
												unshadedDrawInfo,
												unshadedDrawInfoBufferSize + sizeof(uint32_t)),
							   "Failed to resize map unshaded draw info buffer!");
		const uint32_t shadedIndex = PER_FRUSTUM_BUFFER_COUNT * i +
									 (opaque ? PER_FRUSTUM_BUFFER_OPAQUE_MAP_DRAW_INFO_OFFSET
											 : PER_FRUSTUM_BUFFER_MAP_DRAW_INFO_OFFSET);
		ListSet(perFrustumBuffersHandles, shadedIndex, *shadedDrawInfo);
		ListSet(perFrustumBuffersHandles, shadedIndex + 1, *unshadedDrawInfo);

		if (opaque)
		{
			frustums[i].shadedOpaqueMapModelsCullingInfo = lunaGetBufferDeviceAddress(device,
																					  buffer->shadedCullingInfo);
			frustums[i]
					.shadedOpaqueMapModelsUnculledDrawInfo = lunaGetBufferDeviceAddress(device,
																						buffer->unculledShadedDrawInfo);
			frustums[i].shadedOpaqueMapModelsOutputDrawInfo = lunaGetBufferDeviceAddress(device, *shadedDrawInfo);

			frustums[i].unshadedOpaqueMapModelsCullingInfo = lunaGetBufferDeviceAddress(device,
																						buffer->unshadedCullingInfo);
			frustums[i].unshadedOpaqueMapModelsUnculledDrawInfo = lunaGetBufferDeviceAddress(
					device,
					buffer->unculledUnshadedDrawInfo);
			frustums[i].unshadedOpaqueMapModelsOutputDrawInfo = lunaGetBufferDeviceAddress(device, *unshadedDrawInfo);
		} else
		{
			frustums[i].shadedMapModelsCullingInfo = lunaGetBufferDeviceAddress(device, buffer->shadedCullingInfo);
			frustums[i].shadedMapModelsUnculledDrawInfo = lunaGetBufferDeviceAddress(device,
																					 buffer->unculledShadedDrawInfo);
			frustums[i].shadedMapModelsOutputDrawInfo = lunaGetBufferDeviceAddress(device, *shadedDrawInfo);

			frustums[i].unshadedMapModelsCullingInfo = lunaGetBufferDeviceAddress(device, buffer->unshadedCullingInfo);
			frustums[i]
					.unshadedMapModelsUnculledDrawInfo = lunaGetBufferDeviceAddress(device,
																					buffer->unculledUnshadedDrawInfo);
			frustums[i].unshadedMapModelsOutputDrawInfo = lunaGetBufferDeviceAddress(device, *unshadedDrawInfo);
		}
	}
	return VK_SUCCESS;
}

static inline VkResult LoadMapModelsToBuffer(const size_t modelCount,
											 const MapModel *models,
											 const bool opaque,
											 MapModelsBuffer *buffer)
{
	size_t totalVertexCount = 0;
	size_t totalIndexCount = 0;
	size_t totalMaterialCount = 0;
	size_t shadedMaterialCount = 0;
	size_t unshadedMaterialCount = 0;
	for (size_t i = 0; i < modelCount; i++)
	{
		const MapModel *model = models + i;
		if (LoadImage(model->material->texture)->opaque != opaque)
		{
			continue;
		}
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
	maximumCulledInstanceCount = max(maximumCulledInstanceCount, shadedMaterialCount);
	maximumCulledInstanceCount = max(maximumCulledInstanceCount, unshadedMaterialCount);

	const size_t vertexBufferSize = totalVertexCount * sizeof(MapVertex);
	VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffer->vertices, vertexBufferSize),
						   "Failed to resize map vertex buffer!");
	const size_t indexBufferSize = totalIndexCount * sizeof(uint32_t);
	VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffer->indices, indexBufferSize),
						   "Failed to resize map index buffer!");
	const size_t instanceDataBufferSize = totalMaterialCount * sizeof(uint32_t);
	VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffer->instanceData, instanceDataBufferSize),
						   "Failed to resize map instance data buffer!");

	const size_t shadedDrawInfoBufferSize = shadedMaterialCount * sizeof(VkDrawIndexedIndirectCommand);
	const size_t unshadedDrawInfoBufferSize = unshadedMaterialCount * sizeof(VkDrawIndexedIndirectCommand);
	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffer->unculledShadedDrawInfo,
											shadedDrawInfoBufferSize),
						   "Failed to resize unculled map shaded draw info buffer!");
	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffer->unculledUnshadedDrawInfo,
											unshadedDrawInfoBufferSize),
						   "Failed to resize unculled map unshaded draw info buffer!");
	const size_t shadedCullingInfoBufferSize = sizeof(uint32_t) + shadedMaterialCount * sizeof(CullingInfo);
	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffer->shadedCullingInfo,
											shadedCullingInfoBufferSize),
						   "Failed to resize map shaded culling info buffer!");
	const size_t unshadedCullingInfoBufferSize = sizeof(uint32_t) + unshadedMaterialCount * sizeof(CullingInfo);
	VulkanTestReturnResult(lunaResizeBuffer(device,
											commandBuffer,
											&buffer->unshadedCullingInfo,
											unshadedCullingInfoBufferSize),
						   "Failed to resize map unshaded culling info buffer!");
	VulkanTestReturnResult(CreateMapModelDrawInfos(opaque,
												   buffer,
												   shadedDrawInfoBufferSize,
												   unshadedDrawInfoBufferSize),
						   "Failed to create map model draw infos!");

	size_t index = 0;
	VkDeviceSize vertexOffset = 0;
	VkDeviceSize indexOffset = 0;
	size_t shadedMaterialIndex = 0;
	size_t unshadedMaterialIndex = 0;
	MapVertex *vertices = malloc(vertexBufferSize);
	CheckAlloc(vertices);
	uint32_t *indices = malloc(indexBufferSize);
	CheckAlloc(indices);
	uint32_t *textureIndices = malloc(instanceDataBufferSize);
	CheckAlloc(textureIndices);
	VkDrawIndexedIndirectCommand *shadedDrawInfo = malloc(shadedDrawInfoBufferSize);
	CheckAlloc(shadedDrawInfo);
	VkDrawIndexedIndirectCommand *unshadedDrawInfo = malloc(unshadedDrawInfoBufferSize);
	CheckAlloc(unshadedDrawInfo);
	CullingInfo *shadedCullingInfo = malloc(shadedCullingInfoBufferSize);
	CheckAlloc(shadedCullingInfo);
	CullingInfo *unshadedCullingInfo = malloc(unshadedCullingInfoBufferSize);
	CheckAlloc(unshadedCullingInfo);
	for (size_t i = 0; i < modelCount; i++)
	{
		const MapModel *model = models + i;
		if (LoadImage(model->material->texture)->opaque != opaque)
		{
			continue;
		}
		memcpy(vertices + vertexOffset, model->vertices, model->vertexCount * sizeof(MapVertex));
		memcpy(indices + indexOffset, model->indices, model->indexCount * sizeof(uint32_t));
		textureIndices[index] = TextureIndex(model->material->texture);
		switch (model->material->shader)
		{
			case SHADER_SHADED:
				shadedDrawInfo[shadedMaterialIndex].indexCount = model->indexCount;
				shadedDrawInfo[shadedMaterialIndex].instanceCount = 1;
				shadedDrawInfo[shadedMaterialIndex].firstIndex = indexOffset;
				shadedDrawInfo[shadedMaterialIndex].vertexOffset = (int32_t)vertexOffset;
				shadedDrawInfo[shadedMaterialIndex].firstInstance = index;
				shadedCullingInfo[shadedMaterialIndex].position = model->center;
				shadedCullingInfo[shadedMaterialIndex].radius = Vector3_Length(&model->halfExtent);
				shadedCullingInfo[shadedMaterialIndex].castsShadows = model->material->castsShadows;
				shadedMaterialIndex++;
				break;
			case SHADER_UNSHADED:
				unshadedDrawInfo[unshadedMaterialIndex].indexCount = model->indexCount;
				unshadedDrawInfo[unshadedMaterialIndex].instanceCount = 1;
				unshadedDrawInfo[unshadedMaterialIndex].firstIndex = indexOffset;
				unshadedDrawInfo[unshadedMaterialIndex].vertexOffset = (int32_t)vertexOffset;
				unshadedDrawInfo[unshadedMaterialIndex].firstInstance = index;
				unshadedCullingInfo[unshadedMaterialIndex].position = model->center;
				unshadedCullingInfo[unshadedMaterialIndex].radius = Vector3_Length(&model->halfExtent);
				unshadedCullingInfo[unshadedMaterialIndex].castsShadows = model->material->castsShadows;
				unshadedMaterialIndex++;
				break;
			default:
				// Impossible to hit
				free(vertices);
				free(indices);
				free(textureIndices);
				free(shadedDrawInfo);
				free(unshadedDrawInfo);
				free(shadedCullingInfo);
				free(unshadedCullingInfo);
				return VK_ERROR_UNKNOWN;
		}

		vertexOffset += model->vertexCount;
		indexOffset += model->indexCount;
		index++;
	}

	const LunaBufferWriteInfo vertexBufferWriteInfo = {
		.bytes = vertexBufferSize,
		.data = vertices,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffer->vertices, &vertexBufferWriteInfo),
						   "Failed to write data to map vertex buffer!");
	const LunaBufferWriteInfo indexBufferWriteInfo = {
		.bytes = indexBufferSize,
		.data = indices,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffer->indices, &indexBufferWriteInfo),
						   "Failed to write data to map index buffer!");
	const LunaBufferWriteInfo instanceDataBufferWriteInfo = {
		.bytes = instanceDataBufferSize,
		.data = textureIndices,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffer->instanceData,
												 &instanceDataBufferWriteInfo),
						   "Failed to write data to map per-material data buffer!");
	const LunaBufferWriteInfo shadedDrawInfoBufferWriteInfo = {
		.bytes = shadedDrawInfoBufferSize,
		.data = shadedDrawInfo,
		.stageFlags = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffer->unculledShadedDrawInfo,
												 &shadedDrawInfoBufferWriteInfo),
						   "Failed to write data to unculled map shaded draw info buffer!");
	const LunaBufferWriteInfo unshadedDrawInfoBufferWriteInfo = {
		.bytes = unshadedDrawInfoBufferSize,
		.data = unshadedDrawInfo,
		.stageFlags = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffer->unculledUnshadedDrawInfo,
												 &unshadedDrawInfoBufferWriteInfo),
						   "Failed to write data to unculled map unshaded draw info buffer!");
	VulkanTestReturnResult(lunaWriteUintToBuffer(device,
												 commandBuffer,
												 buffer->shadedCullingInfo,
												 0,
												 shadedMaterialCount,
												 NULL),
						   "Failed to write map shaded culling info count to buffer!");
	const LunaBufferWriteInfo shadedCullingInfoBufferWriteInfo = {
		.bytes = shadedMaterialCount * sizeof(CullingInfo),
		.data = shadedCullingInfo,
		.offset = sizeof(uint32_t),
		.stageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffer->shadedCullingInfo,
												 &shadedCullingInfoBufferWriteInfo),
						   "Failed to write data to map shaded culling info buffer!");
	VulkanTestReturnResult(lunaWriteUintToBuffer(device,
												 commandBuffer,
												 buffer->unshadedCullingInfo,
												 0,
												 unshadedMaterialCount,
												 NULL),
						   "Failed to write map unshaded culling info count to buffer!");
	const LunaBufferWriteInfo unshadedCullingInfoBufferWriteInfo = {
		.bytes = unshadedMaterialCount * sizeof(CullingInfo),
		.data = unshadedCullingInfo,
		.offset = sizeof(uint32_t),
		.stageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffer->unshadedCullingInfo,
												 &unshadedCullingInfoBufferWriteInfo),
						   "Failed to write data to map unshaded culling info buffer!");

	free(vertices);
	free(indices);
	free(textureIndices);
	free(shadedDrawInfo);
	free(unshadedDrawInfo);
	free(shadedCullingInfo);
	free(unshadedCullingInfo);

	VulkanTestReturnResult(WriteFrustumsBuffer(), "Failed to write frustums buffer!");

	const LunaMultiBufferMemoryBarrier perFrustumBuffesBarrier = {
		.sourceStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		.sourceAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		.destinationAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
		.bufferCount = perFrustumBuffersHandles.length,
		.buffers = (LunaBuffer *)perFrustumBuffersHandles.data->pointerData,
	};
	const LunaMultiBufferMemoryBarrier cullingInfoBarrier = {
		.sourceStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		.sourceAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		.destinationAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
		.bufferCount = 2,
		.buffers = (LunaBuffer[]){buffer->shadedCullingInfo, buffer->unshadedCullingInfo},
	};
	const LunaDependencyInfo dependencyInfo = {
		.multiBufferMemoryBarrierCount = 2,
		.multiBufferMemoryBarriers = (LunaMultiBufferMemoryBarrier[]){perFrustumBuffesBarrier, cullingInfoBarrier},
	};
	VulkanTestReturnResult(lunaPipelineBarrier(device, commandBuffer, &dependencyInfo),
						   "Failed to insert pipeline barrier after writing map data!");

	return VK_SUCCESS;
}

static inline VkResult UpdateMapInstanceData(const Map *map, const bool opaque, const MapModelsBuffer *buffer)
{
	const size_t materialCount = lunaGetBufferSize(buffer->instanceData) / sizeof(uint32_t);
	uint32_t textureIndices[materialCount];
	size_t materialIndex = 0;
	for (size_t i = 0; i < map->modelCount; i++)
	{
		const MapModel *model = &map->models[i];
		if (LoadImage(model->material->texture)->opaque != opaque)
		{
			continue;
		}
		textureIndices[materialIndex] = TextureIndex(model->material->texture);
		materialIndex++;
	}
	const LunaBufferWriteInfo instanceDataBufferWriteInfo = {
		.bytes = lunaGetBufferSize(buffer->instanceData),
		.data = textureIndices,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffer->instanceData,
												 &instanceDataBufferWriteInfo),
						   "Failed to update map instance data buffer!");

	return VK_SUCCESS;
}

static inline VkResult LoadLightmap(const Map *map)
{
	if (lightmap != LUNA_NULL_HANDLE)
	{
		lunaDestroyImage(device, lightmap);
		lightmap = LUNA_NULL_HANDLE;
	}
	const VkPipelineStageFlags2 waitStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	const LunaCommandBufferSubmitInfo submitInfo = {
		.queue = queue,
		.waitSemaphoreCount = 1,
		.waitSemaphores = &semaphore,
		.waitDstStageMasks = &waitStage,
		.signalSemaphoreCount = 1,
		.signalSemaphores = &semaphore,
	};
	const LunaImageCreationInfo imageCreationInfo = {
		.format = VK_FORMAT_R16G16B16A16_SFLOAT,
		.width = map->lightmapWidth,
		.height = map->lightmapHeight,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.writeInfo.bytes = map->lightmapWidth * map->lightmapHeight * sizeof(_Float16) * 4,
		.writeInfo.pixels = map->lightmapPixels,
		.writeInfo.sourceStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		.writeInfo.destinationStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		.writeInfo.destinationAccessMask = VK_ACCESS_SHADER_READ_BIT,
		.writeInfo.submitInfo = &submitInfo,
		.sampler = textureSamplers.linearNoRepeatNoAnisotropy,
	};
	VulkanTestReturnResult(lunaCreateImage(device, secondaryCommandBuffer, &imageCreationInfo, &lightmap),
						   "Failed to create texture!");

	const LunaDescriptorImageInfo imageInfo = {
		.image = lightmap,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};
	const LunaWriteDescriptorSet writeDescriptor = {
		.descriptorSet = descriptorSets.common.set,
		.bindingName = "Lightmap",
		.descriptorCount = 1,
		.imageInfos = &imageInfo,
	};
	lunaWriteDescriptorSets(device, 1, &writeDescriptor);

	return VK_SUCCESS;
}

static inline float GetMaxLightDistance(const Light *light)
{
	if (light->type == LIGHT_TYPE_DIRECTIONAL)
	{
		return 0;
	}

	const float a = light->quadraticAttenuation;
	const float b = light->attenuationMultiplier * light->linearAttenuation;
	const float c = -light->attenuationMultiplier *
					light->attenuationMultiplier *
					(light->brightness * 256 - light->constantAttenuation);
	const float val = b * b - 4 * a * c;
	if (val <= 0)
	{
		return 0;
	}
	return (sqrtf(val) - b) / (2 * a);
}

static inline VkResult CreatePerFrustumBuffers()
{
	for (uint32_t i = 0; i < perFrustumBuffersHandles.length; i++)
	{
		lunaDestroyBuffer(device, (LunaBuffer)ListGetPointer(perFrustumBuffersHandles, i));
	}

	ListFree(perFrustumBuffersHandles);
	ListInit(perFrustumBuffersHandles, LIST_POINTER);

	const LunaBufferCreationInfo drawInfoBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	const LunaBufferCreationInfo actorWallDrawInfoBufferCreationInfo = {
		.size = sizeof(VkDrawIndirectCommand),
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	const LunaBufferCreationInfo instanceIndicesBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	List *drawInfoBufferLists[] = {
		&buffers.actorModels.shadedDrawInfo,
		&buffers.actorModels.unshadedDrawInfo,
		&buffers.actorWalls.shadedDrawInfo,
		&buffers.actorWalls.unshadedDrawInfo,
		&buffers.opaqueMap.shadedDrawInfo,
		&buffers.opaqueMap.unshadedDrawInfo,
		&buffers.map.shadedDrawInfo,
		&buffers.map.unshadedDrawInfo,
	};
	List *instanceIndicesLists[] = {
		&buffers.actorModels.shadedInstanceIndices,
		&buffers.actorModels.unshadedInstanceIndices,
		&buffers.actorWalls.shadedInstanceIndices,
		&buffers.actorWalls.unshadedInstanceIndices,
	};
	for (size_t j = 0; j < sizeof(drawInfoBufferLists) / sizeof(*drawInfoBufferLists); j++)
	{
		ListFree(*drawInfoBufferLists[j]);
		ListInit(*drawInfoBufferLists[j], LIST_POINTER);
	}
	for (size_t j = 0; j < sizeof(instanceIndicesLists) / sizeof(*instanceIndicesLists); j++)
	{
		ListFree(*instanceIndicesLists[j]);
		ListInit(*instanceIndicesLists[j], LIST_POINTER);
	}
	for (uint32_t i = 0; i < frustumCount; i++)
	{
		for (size_t j = 0; j < sizeof(drawInfoBufferLists) / sizeof(*drawInfoBufferLists); j++)
		{
			List *list = drawInfoBufferLists[j];
			LunaBuffer *buffer = ListAdd(*list, LUNA_NULL_HANDLE);
			if (list == &buffers.actorWalls.shadedDrawInfo || list == &buffers.actorWalls.unshadedDrawInfo)
			{
				VulkanTestReturnResult(lunaCreateBuffer(device, &actorWallDrawInfoBufferCreationInfo, buffer),
									   "Failed to create draw info buffer!");

			} else
			{
				VulkanTestReturnResult(lunaCreateBuffer(device, &drawInfoBufferCreationInfo, buffer),
									   "Failed to create draw info buffer!");
			}
			ListAdd(perFrustumBuffersHandles, *buffer);
		}
		for (size_t j = 0; j < sizeof(instanceIndicesLists) / sizeof(*instanceIndicesLists); j++)
		{
			LunaBuffer *buffer = ListAdd(*(instanceIndicesLists[j]), LUNA_NULL_HANDLE);
			VulkanTestReturnResult(lunaCreateBuffer(device, &instanceIndicesBufferCreationInfo, buffer),
								   "Failed to create instance indices buffer!");
			ListAdd(perFrustumBuffersHandles, *buffer);
		}
	}

	return VK_SUCCESS;
}

static inline VkResult LoadLights(const Map *map)
{
	if (GetState()->options.shadowMapQuality == SHADOW_MAP_RESOLUTION_DISABLED)
	{
		AvxAlignedFree(frustums);
		frustums = AvxAlignedCalloc(sizeof(FrustumCullingData));
		CheckAlloc(frustums);
		frustumCount = 1; // Just the camera's frustum
		VulkanTestReturnResult(CreatePerFrustumBuffers(), "Failed to create per-frustum buffers!");
		VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.frustums, sizeof(FrustumCullingData)),
							   "Failed to resize frustums buffer!");
		VulkanTestReturnResult(WriteFrustumsBuffer(), "Failed to write data to frustums buffer!");
		const LunaDescriptorBufferInfo frustumsBufferInfo = {
			.buffer = buffers.frustums,
		};
		const LunaWriteDescriptorSet frustumsDescriptorWrite = {
			.descriptorSet = descriptorSets.culling.set,
			.bindingName = "Frustum Culling Data",
			.descriptorCount = 1,
			.bufferInfos = &frustumsBufferInfo,
		};

		lunaWriteDescriptorSets(device, 1, &frustumsDescriptorWrite);

		lightCount = 0;
		VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.uniforms.lights, 0),
							   "Failed to resize lights buffer!");

		if (!UpdateLightCount())
		{
			return VK_ERROR_UNKNOWN;
		}

		return VK_SUCCESS;
	}

	static const float LIGHT_NEAR_PLANE = 0.01f;

	AvxAlignedFree(frustums);
	frustums = AvxAlignedCalloc(sizeof(FrustumCullingData) * (map->lightCount * 6 + 1));
	CheckAlloc(frustums);
	uint32_t frustumIndex = map->directionalLight == NULL ? 1 : 5;
	uint32_t spotLightIndex = 0;
	uint32_t pointLightIndex = 0;
	for (lightCount = 0; lightCount < map->lightCount; lightCount++)
	{
		mat4 transformMatrix;
		Light *light = &map->lights[lightCount];
		light->maxDistance = GetMaxLightDistance(light);
		switch (light->type)
		{
			case LIGHT_TYPE_SPOT:
			{
				light->shadowMapIndex = spotLightIndex++;
				versor rotationQuat;
				QUAT_TO_VERSOR(light->transform.rotation, rotationQuat);
				versor rotationOffset;
				glm_quatv(rotationOffset, GLM_PIf, GLM_XUP);
				glm_quat_mul(rotationQuat, rotationOffset, rotationQuat);
				glm_quat_look(VECTOR3_TO_VEC3(light->transform.position),
							  rotationQuat,
							  frustums[frustumIndex].viewMatrix);
				mat4 projectionMatrix;
				glm_perspective_lh_zo(glm_rad(2 * light->fadingAngle),
									  1,
									  light->maxDistance,
									  LIGHT_NEAR_PLANE,
									  projectionMatrix);
				glm_mat4_mul(projectionMatrix, frustums[frustumIndex].viewMatrix, transformMatrix);

				frustums[frustumIndex].nearPlane = LIGHT_NEAR_PLANE;
				frustums[frustumIndex].farPlane = light->maxDistance;
				frustums[frustumIndex].frustumPlanes[1] = 1 / Vector2Length(v2(projectionMatrix[0][0], 1));
				frustums[frustumIndex].frustumPlanes[3] = 1 / Vector2Length(v2(projectionMatrix[1][1], 1));
				frustums[frustumIndex].frustumPlanes[0] = projectionMatrix[0][0] *
														  frustums[frustumIndex].frustumPlanes[1];
				frustums[frustumIndex].frustumPlanes[2] = projectionMatrix[1][1] *
														  frustums[frustumIndex].frustumPlanes[3];
				frustumIndex++;
			}
			break;
			case LIGHT_TYPE_POINT:
			{
				light->shadowMapIndex = pointLightIndex++;
				glm_perspective_lh_zo(glm_rad(90), 1, light->maxDistance, LIGHT_NEAR_PLANE, transformMatrix);

				mat3 transforms[6] = {
					{{0, 0, -1}, {0, 1, 0}, {1, 0, 0}},
					{{0, 0, 1}, {0, 1, 0}, {-1, 0, 0}},
					{{-1, 0, 0}, {0, 0, -1}, {0, -1, 0}},
					{{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}},
					{{-1, 0, 0}, {0, 1, 0}, {0, 0, -1}},
					{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
				};
				vec3 negativeLightPosition = {
					-light->transform.position.x,
					-light->transform.position.y,
					-light->transform.position.z,
				};
				for (uint32_t j = 0; j < 6; j++)
				{
					glm_mat4_ins3(transforms[j], frustums[frustumIndex].viewMatrix);
					glm_translate(frustums[frustumIndex].viewMatrix, negativeLightPosition);

					frustums[frustumIndex].nearPlane = LIGHT_NEAR_PLANE;
					frustums[frustumIndex].farPlane = light->maxDistance;
					frustums[frustumIndex].frustumPlanes[1] = 1 / Vector2Length(v2(transformMatrix[0][0], 1));
					frustums[frustumIndex].frustumPlanes[3] = 1 / Vector2Length(v2(transformMatrix[1][1], 1));
					frustums[frustumIndex].frustumPlanes[0] = transformMatrix[0][0] *
															  frustums[frustumIndex].frustumPlanes[1];
					frustums[frustumIndex].frustumPlanes[2] = transformMatrix[1][1] *
															  frustums[frustumIndex].frustumPlanes[3];
					frustumIndex++;
				}
			}
			break;
			default:
				continue;
		}

		// The allocation for lights is not aligned so we just memcpy
		memcpy(light->transformMatrix, transformMatrix, sizeof(mat4));
	}

	frustumCount = frustumIndex;
	VulkanTestReturnResult(CreatePerFrustumBuffers(), "Failed to create per-frustum buffers!");

	const size_t frustumBufferSize = sizeof(FrustumCullingData) * frustumCount;
	VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.frustums, frustumBufferSize),
						   "Failed to resize frustums buffer!");
	VulkanTestReturnResult(WriteFrustumsBuffer(), "Failed to write data to frustums buffer!");
	const LunaDescriptorBufferInfo frustumsBufferInfo = {
		.buffer = buffers.frustums,
	};
	const LunaWriteDescriptorSet frustumsDescriptorWrite = {
		.descriptorSet = descriptorSets.culling.set,
		.bindingName = "Frustum Culling Data",
		.descriptorCount = 1,
		.bufferInfos = &frustumsBufferInfo,
	};

	const size_t lightsBufferSize = sizeof(float) * 4 + sizeof(mat4) * 4 + sizeof(Light) * lightCount;
	VulkanTestReturnResult(lunaResizeBuffer(device, commandBuffer, &buffers.uniforms.lights, lightsBufferSize),
						   "Failed to resize lights buffer!");

	const LunaBufferWriteInfo bufferWriteInfo = {
		.bytes = lightCount * sizeof(Light),
		.data = map->lights,
		.offset = sizeof(float) * 4 + sizeof(mat4) * 4,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.uniforms.lights, &bufferWriteInfo),
						   "Failed to write lights data to buffer!");

	const LunaDescriptorBufferInfo lightsBufferInfo = {
		.buffer = buffers.uniforms.lights,
	};
	const LunaWriteDescriptorSet lightsDescriptorWrite = {
		.descriptorSet = descriptorSets.common.set,
		.bindingName = "Lights",
		.descriptorCount = 1,
		.bufferInfos = &lightsBufferInfo,
	};
	lunaWriteDescriptorSets(device, 2, (LunaWriteDescriptorSet[]){frustumsDescriptorWrite, lightsDescriptorWrite});

	if (!UpdateLightCount())
	{
		return VK_ERROR_UNKNOWN;
	}

	return VK_SUCCESS;
}

static inline VkResult CreateShadowMaps(const Map *map)
{
	VulkanTestReturnResult(CreateShadowMapRenderPass(map), "Failed to create shadow map render pass!");
	VulkanTestReturnResult(CreateDepthGraphicsPipelines(), "Failed to create shadow map graphics pipelines!");

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

static inline VkResult DrawModelBuffer(const ModelBuffer *buffer,
									   const LunaGraphicsPipeline shadedPipeline,
									   const LunaGraphicsPipeline unshadedPipeline,
									   const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
									   const char *name)
{
	const size_t shadedDrawCount = lunaGetBufferSize(buffer->shadedDrawInfo) / sizeof(VkDrawIndexedIndirectCommand);
	const size_t unshadedDrawCount = lunaGetBufferSize(buffer->unshadedDrawInfo) / sizeof(VkDrawIndexedIndirectCommand);

	if (shadedDrawCount != 0 || unshadedDrawCount != 0)
	{
		VulkanTestReturnResult(lunaBindVertexBuffers(device,
													 commandBuffer,
													 (LunaBuffer[]){buffer->vertices, buffer->instanceData},
													 0,
													 2),
							   "Failed to bind %s vertex buffers!",
							   name);
		VulkanTestReturnResult(lunaBindIndexBuffer(device, commandBuffer, buffer->indices, VK_INDEX_TYPE_UINT32),
							   "Failed to bind %s index buffer!",
							   name);
	}

	if (shadedDrawCount != 0)
	{
		const LunaDrawIndexedIndirectInfo drawInfo = {
			.pipeline = shadedPipeline,
			.pipelineBindInfo = pipelineBindInfo,
			.buffer = buffer->shadedDrawInfo,
			.drawCount = shadedDrawCount,
		};
		VulkanTestReturnResult(lunaDrawIndexedIndirect(device, commandBuffer, &drawInfo),
							   "Failed to draw shaded %s!",
							   name);
	}

	if (unshadedDrawCount != 0)
	{
		const LunaDrawIndexedIndirectInfo drawInfo = {
			.pipeline = unshadedPipeline,
			.pipelineBindInfo = pipelineBindInfo,
			.buffer = buffer->unshadedDrawInfo,
			.drawCount = unshadedDrawCount,
		};
		VulkanTestReturnResult(lunaDrawIndexedIndirect(device, commandBuffer, &drawInfo),
							   "Failed to draw unshaded %s!",
							   name);
	}

	return VK_SUCCESS;
}

static inline VkResult DrawMapModelsBuffer(const MapModelsBuffer *buffer,
										   const uint32_t frustumIndex,
										   const LunaGraphicsPipeline shadedPipeline,
										   const LunaGraphicsPipeline unshadedPipeline,
										   const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
										   const bool pushConstants,
										   const char *name)
{
	const size_t shadedDrawCount = lunaGetBufferSize(buffer->unculledShadedDrawInfo) /
								   sizeof(VkDrawIndexedIndirectCommand);
	const size_t unshadedDrawCount = lunaGetBufferSize(buffer->unculledUnshadedDrawInfo) /
									 sizeof(VkDrawIndexedIndirectCommand);

	if (shadedDrawCount != 0 || unshadedDrawCount != 0)
	{
		VulkanTestReturnResult(lunaBindVertexBuffers(device,
													 commandBuffer,
													 (LunaBuffer[]){buffer->vertices, buffer->instanceData},
													 0,
													 2),
							   "Failed to bind %s vertex buffers!",
							   name);
		VulkanTestReturnResult(lunaBindIndexBuffer(device, commandBuffer, buffer->indices, VK_INDEX_TYPE_UINT32),
							   "Failed to bind %s index buffer!",
							   name);

		if (pushConstants)
		{
			VulkanTestReturnResult(lunaPushConstants(device, commandBuffer, shadedPipeline),
								   "Failed to push constants for %s pipeline!",
								   name);
		}
	}

	if (shadedDrawCount != 0)
	{
		const LunaDrawIndexedIndirectCountInfo drawInfo = {
			.pipeline = shadedPipeline,
			.pipelineBindInfo = pipelineBindInfo,
			.buffer = (LunaBuffer)ListGetPointer(buffer->shadedDrawInfo, frustumIndex),
			.maxDrawCount = shadedDrawCount,
		};
		VulkanTestReturnResult(lunaDrawIndexedIndirectCount(device, commandBuffer, &drawInfo),
							   "Failed to draw shaded %s!",
							   name);
	}

	if (unshadedDrawCount != 0)
	{
		const LunaDrawIndexedIndirectCountInfo drawInfo = {
			.pipeline = unshadedPipeline,
			.pipelineBindInfo = pipelineBindInfo,
			.buffer = (LunaBuffer)ListGetPointer(buffer->unshadedDrawInfo, frustumIndex),
			.maxDrawCount = unshadedDrawCount,
		};
		VulkanTestReturnResult(lunaDrawIndexedIndirectCount(device, commandBuffer, &drawInfo),
							   "Failed to draw unshaded %s!",
							   name);
	}

	return VK_SUCCESS;
}

static inline VkResult DrawMap(const uint32_t frustumIndex,
							   const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
							   const bool depthPrepass,
							   const LunaGraphicsPipeline opaqueShadowMapsPipeline,
							   const LunaGraphicsPipeline shadowMapsPipeline)
{
	if (depthPrepass)
	{
		shadowMapPushConstants.lightType = -1u;
		VulkanTestReturnResult(DrawMapModelsBuffer(&buffers.opaqueMap,
												   0,
												   pipelines.depthPrepass.opaqueMap,
												   pipelines.depthPrepass.opaqueMap,
												   pipelineBindInfo,
												   true,
												   "opaque map"),
							   "Failed to draw opaque map!");
		VulkanTestReturnResult(DrawMapModelsBuffer(&buffers.map,
												   0,
												   pipelines.depthPrepass.map,
												   pipelines.depthPrepass.map,
												   pipelineBindInfo,
												   true,
												   "map"),
							   "Failed to draw map!");
	} else if (shadowMapsPipeline != LUNA_NULL_HANDLE)
	{
		VulkanTestReturnResult(DrawMapModelsBuffer(&buffers.opaqueMap,
												   frustumIndex,
												   opaqueShadowMapsPipeline,
												   opaqueShadowMapsPipeline,
												   pipelineBindInfo,
												   true,
												   "opaque map"),
							   "Failed to draw opaque map!");
		VulkanTestReturnResult(DrawMapModelsBuffer(&buffers.map,
												   frustumIndex,
												   shadowMapsPipeline,
												   shadowMapsPipeline,
												   pipelineBindInfo,
												   true,
												   "map"),
							   "Failed to draw map!");
	} else
	{
		VulkanTestReturnResult(DrawMapModelsBuffer(&buffers.opaqueMap,
												   frustumIndex,
												   pipelines.shadedMap,
												   pipelines.unshadedMap,
												   pipelineBindInfo,
												   false,
												   "opaque map"),
							   "Failed to draw opaque map!");
		VulkanTestReturnResult(DrawMapModelsBuffer(&buffers.map,
												   frustumIndex,
												   pipelines.shadedMap,
												   pipelines.unshadedMap,
												   pipelineBindInfo,
												   false,
												   "map"),
							   "Failed to draw map!");
	}

	return VK_SUCCESS;
}

static inline VkResult DrawActors(const uint32_t frustumIndex,
								  const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
								  const LunaGraphicsPipeline modelShadowMapsPipeline,
								  const LunaGraphicsPipeline wallShadowMapsPipeline)
{
	const LunaBuffer shadedDrawInfo = (LunaBuffer)ListGetPointer(buffers.actorModels.shadedDrawInfo, frustumIndex);
	const LunaBuffer unshadedDrawInfo = (LunaBuffer)ListGetPointer(buffers.actorModels.unshadedDrawInfo, frustumIndex);
	const size_t shadedDrawCount = lunaGetBufferSize(shadedDrawInfo) / sizeof(VkDrawIndexedIndirectCommand);
	const size_t unshadedDrawCount = lunaGetBufferSize(unshadedDrawInfo) / sizeof(VkDrawIndexedIndirectCommand);

	if (shadedDrawCount != 0 || unshadedDrawCount != 0)
	{
		VulkanTestReturnResult(lunaBindVertexBuffers(device, commandBuffer, &buffers.actorModels.vertices, 0, 1),
							   "Failed to bind actor models vertex buffers!");
		VulkanTestReturnResult(lunaBindIndexBuffer(device,
												   commandBuffer,
												   buffers.actorModels.indices,
												   VK_INDEX_TYPE_UINT32),
							   "Failed to bind actor models index buffer!");

		if (modelShadowMapsPipeline != LUNA_NULL_HANDLE)
		{
			VulkanTestReturnResult(lunaPushConstants(device, commandBuffer, modelShadowMapsPipeline),
								   "Failed to push constants for model actor shadow map pipeline!");
		}

		if (shadedDrawCount != 0)
		{
			const LunaBuffer instanceIndicesBuffer = (LunaBuffer)
					ListGetPointer(buffers.actorModels.shadedInstanceIndices, frustumIndex);
			VulkanTestReturnResult(lunaBindVertexBuffers(device, commandBuffer, &instanceIndicesBuffer, 1, 1),
								   "Failed to bind shaded actor model instance indices buffer!");
			const LunaDrawIndexedIndirectInfo drawInfo = {
				.pipeline = modelShadowMapsPipeline != LUNA_NULL_HANDLE ? modelShadowMapsPipeline
																		: pipelines.shadedActorModel,
				.pipelineBindInfo = pipelineBindInfo,
				.buffer = shadedDrawInfo,
				.drawCount = shadedDrawCount,
			};
			VulkanTestReturnResult(lunaDrawIndexedIndirect(device, commandBuffer, &drawInfo),
								   "Failed to draw shaded actor models!");
		}

		if (unshadedDrawCount != 0)
		{
			const LunaBuffer instanceIndicesBuffer = (LunaBuffer)
					ListGetPointer(buffers.actorModels.unshadedInstanceIndices, frustumIndex);
			VulkanTestReturnResult(lunaBindVertexBuffers(device, commandBuffer, &instanceIndicesBuffer, 1, 1),
								   "Failed to bind unshaded actor model instance indices buffer!");
			const LunaDrawIndexedIndirectInfo drawInfo = {
				.pipeline = modelShadowMapsPipeline != LUNA_NULL_HANDLE ? modelShadowMapsPipeline
																		: pipelines.unshadedActorModel,
				.pipelineBindInfo = pipelineBindInfo,
				.buffer = unshadedDrawInfo,
				.drawCount = unshadedDrawCount,
			};
			VulkanTestReturnResult(lunaDrawIndexedIndirect(device, commandBuffer, &drawInfo),
								   "Failed to draw unshaded actor models!");
		}
	}

	if (buffers.actorWalls.shadedInstanceCount != 0 || buffers.actorWalls.unshadedInstanceCount != 0)
	{
		VulkanTestReturnResult(lunaBindVertexBuffers(device, commandBuffer, &buffers.actorWalls.vertices, 0, 1),
							   "Failed to bind actor wall vertex buffer!");

		if (wallShadowMapsPipeline != LUNA_NULL_HANDLE)
		{
			VulkanTestReturnResult(lunaPushConstants(device, commandBuffer, wallShadowMapsPipeline),
								   "Failed to push constants for wall actor shadow map pipeline!");
		}

		if (buffers.actorWalls.shadedInstanceCount != 0)
		{
			const LunaBuffer drawInfoBuffer = (LunaBuffer)ListGetPointer(buffers.actorWalls.shadedDrawInfo,
																		 frustumIndex);
			const LunaBuffer instanceIndicesBuffer = (LunaBuffer)
					ListGetPointer(buffers.actorWalls.shadedInstanceIndices, frustumIndex);
			VulkanTestReturnResult(lunaBindVertexBuffers(device, commandBuffer, &instanceIndicesBuffer, 1, 1),
								   "Failed to bind shaded actor wall instance indices buffer!");
			const LunaDrawIndirectInfo drawInfo = {
				.pipeline = wallShadowMapsPipeline != LUNA_NULL_HANDLE ? wallShadowMapsPipeline
																	   : pipelines.shadedActorWall,
				.pipelineBindInfo = pipelineBindInfo,
				.buffer = drawInfoBuffer,
				.drawCount = 1,
			};
			VulkanTestReturnResult(lunaDrawIndirect(device, commandBuffer, &drawInfo),
								   "Failed to draw shaded actor walls!");
		}

		if (buffers.actorWalls.unshadedInstanceCount != 0)
		{
			const LunaBuffer drawInfoBuffer = (LunaBuffer)ListGetPointer(buffers.actorWalls.unshadedDrawInfo,
																		 frustumIndex);
			const LunaBuffer instanceIndicesBuffer = (LunaBuffer)
					ListGetPointer(buffers.actorWalls.unshadedInstanceIndices, frustumIndex);
			VulkanTestReturnResult(lunaBindVertexBuffers(device, commandBuffer, &instanceIndicesBuffer, 1, 1),
								   "Failed to bind unshaded actor wall instance indices buffer!");
			const LunaDrawIndirectInfo drawInfo = {
				.pipeline = wallShadowMapsPipeline != LUNA_NULL_HANDLE ? wallShadowMapsPipeline
																	   : pipelines.unshadedActorWall,
				.pipelineBindInfo = pipelineBindInfo,
				.buffer = drawInfoBuffer,
				.drawCount = 1,
			};
			VulkanTestReturnResult(lunaDrawIndirect(device, commandBuffer, &drawInfo),
								   "Failed to draw unshaded actor walls!");
		}
	}

	return VK_SUCCESS;
}

// TODO: Switch this to use the shared DrawModel function
static inline VkResult DrawPlayer(const Map *map, const LunaGraphicsPipelineBindInfo *pipelineBindInfo)
{
	const size_t shadedDrawCount = lunaGetBufferSize(buffers.player.buffers.shadedDrawInfo) /
								   sizeof(VkDrawIndexedIndirectCommand);
	const size_t unshadedDrawCount = lunaGetBufferSize(buffers.player.buffers.unshadedDrawInfo) /
									 sizeof(VkDrawIndexedIndirectCommand);

	if (shadedDrawCount != 0 || unshadedDrawCount != 0)
	{
		JPH_RMat44 transformMatrix;
		JPH_CharacterVirtual_GetWorldTransform(map->player.joltCharacter, &transformMatrix);
		for (uint32_t slotIndex = 0; slotIndex < buffers.player.modelDefinition->materialSlotCount; slotIndex++)
		{
			memcpy(buffers.player.instanceData + slotIndex, &transformMatrix, sizeof(transformMatrix));
		}
		const LunaBufferWriteInfo instanceDataWriteInfo = {
			.bytes = sizeof(ModelInstanceData) * buffers.player.modelDefinition->materialSlotCount,
			.data = buffers.player.instanceData,
			.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		};
		VulkanTestReturnResult(lunaWriteDataToBuffer(device,
													 commandBuffer,
													 buffers.player.buffers.instanceData,
													 &instanceDataWriteInfo),
							   "Failed to write player instance data to buffer!");

		VulkanTestReturnResult(lunaBindVertexBuffers(device,
													 commandBuffer,
													 (LunaBuffer[]){buffers.player.buffers.vertices,
																	buffers.player.buffers.instanceData},
													 0,
													 2),
							   "Failed to bind player vertex buffers!");
		VulkanTestReturnResult(lunaBindIndexBuffer(device,
												   commandBuffer,
												   buffers.player.buffers.indices,
												   VK_INDEX_TYPE_UINT32),
							   "Failed to bind player index buffer!");
	}

	if (shadedDrawCount != 0)
	{
		const LunaDrawIndexedIndirectInfo drawInfo = {
			.pipeline = pipelines.shadedModel,
			.pipelineBindInfo = pipelineBindInfo,
			.buffer = buffers.player.buffers.shadedDrawInfo,
			.drawCount = shadedDrawCount,
		};
		VulkanTestReturnResult(lunaDrawIndexedIndirect(device, commandBuffer, &drawInfo),
							   "Failed to draw shaded portions of player model!");
	}

	if (unshadedDrawCount != 0)
	{
		const LunaDrawIndexedIndirectInfo drawInfo = {
			.pipeline = pipelines.unshadedModel,
			.pipelineBindInfo = pipelineBindInfo,
			.buffer = buffers.player.buffers.unshadedDrawInfo,
			.drawCount = unshadedDrawCount,
		};
		VulkanTestReturnResult(lunaDrawIndexedIndirect(device, commandBuffer, &drawInfo),
							   "Failed to draw unshaded portions of player model!");
	}

	return VK_SUCCESS;
}

static inline VkResult DrawViewmodel(const LunaGraphicsPipelineBindInfo *pipelineBindInfo)
{
	return DrawModelBuffer(&buffers.viewmodel,
						   pipelines.shadedModel,
						   pipelines.unshadedMap,
						   pipelineBindInfo,
						   "viewmodel");
}

static inline VkResult DrawDebugRenderer(const LunaGraphicsPipelineBindInfo *pipelineBindInfo)
{
#ifdef JPH_DEBUG_RENDERER
	VulkanTestReturnResult(ResizeDebugDrawBuffers(), "Failed to resize debug draw buffer!");

	if (buffers.debugDrawLines.bytesUsed != 0)
	{
		const LunaBufferWriteInfo writeInfo = {
			.bytes = buffers.debugDrawLines.bytesUsed,
			.data = buffers.debugDrawLines.vertices,
			.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		};
		VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.debugDrawLines.buffer, &writeInfo),
							   "Failed to write debug Jolt debug renderer lines to buffer!");
		const LunaDrawInfo drawInfo = {
			.pipeline = pipelines.debugDrawLines,
			.pipelineBindInfo = pipelineBindInfo,
			.vertexCount = buffers.debugDrawLines.bytesUsed / sizeof(DebugDrawVertex),
			.instanceCount = 1,
		};
		VulkanTestReturnResult(lunaDrawBuffer(device, commandBuffer, buffers.debugDrawLines.buffer, &drawInfo),
							   "Failed to draw Jolt debug renderer lines!");
	}
	if (buffers.debugDrawTriangles.bytesUsed != 0)
	{
		const LunaBufferWriteInfo writeInfo = {
			.bytes = buffers.debugDrawTriangles.bytesUsed,
			.data = buffers.debugDrawTriangles.vertices,
			.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		};
		VulkanTestReturnResult(lunaWriteDataToBuffer(device,
													 commandBuffer,
													 buffers.debugDrawTriangles.buffer,
													 &writeInfo),
							   "Failed to write Jolt debug renderer triangles to buffer!");
		const LunaDrawInfo drawInfo = {
			.pipeline = pipelines.debugDrawTriangles,
			.pipelineBindInfo = pipelineBindInfo,
			.vertexCount = buffers.debugDrawTriangles.bytesUsed / sizeof(DebugDrawVertex),
			.instanceCount = 1,
		};
		VulkanTestReturnResult(lunaDrawBuffer(device, commandBuffer, buffers.debugDrawTriangles.buffer, &drawInfo),
							   "Failed to draw Jolt debug renderer triangles!");
	}
#else
	(void)pipelineBindInfo;
#endif

	return VK_SUCCESS;
}

static inline VkResult UpdateShadowMaps(const Map *map)
{
	if (GetState()->options.shadowMapQuality == SHADOW_MAP_RESOLUTION_DISABLED)
	{
		return VK_SUCCESS;
	}

	const VkCommandBuffer vkCommandBuffer = lunaGetVkCommandBuffer(commandBuffer);

	const LunaDescriptorSetBindInfo descriptorSetBindInfo = {
		.descriptorSetCount = 1,
		.descriptorSets = &descriptorSets.common.set,
	};
	VulkanTestReturnResult(lunaBindDescriptorSets(device,
												  commandBuffer,
												  pipelines.shadowMaps.map,
												  &descriptorSetBindInfo),
						   "Failed to bind descriptor sets!");
	LunaGraphicsPipelineBindInfo pipelineBindInfo = {};
	VkViewport viewport = {.maxDepth = 1};
	const LunaViewportBindInfo viewportBindInfo = {
		.viewportCount = 1,
		.viewports = &viewport,
	};
	VkRect2D scissor = {};
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
	const VkClearValue depthClearValue = {
		.depthStencil.depth = 0,
	};

	uint32_t frustumIndex = map->directionalLight == NULL ? 1 : 5;
	uint32_t previousSize = 0;
	uint32_t framebufferIndex = 0;
	for (shadowMapPushConstants.lightIndex = 0; shadowMapPushConstants.lightIndex < lightCount;
		 shadowMapPushConstants.lightIndex++)
	{
		const Light *light = &map->lights[shadowMapPushConstants.lightIndex];
		const uint32_t size = ShadowMapResolution(light->type);
		shadowMapPushConstants.lightType = light->type;

		const VkExtent2D extent = {
			.width = size,
			.height = size,
		};

		if (size != previousSize)
		{
			viewport.width = (float)extent.width;
			viewport.height = (float)extent.height;
			scissor.extent = extent;
			pipelineBindInfo.dynamicStateCount = sizeof(dynamicStateBindInfos) / sizeof(*dynamicStateBindInfos);
			pipelineBindInfo.dynamicStates = dynamicStateBindInfos;
		} else
		{
			pipelineBindInfo.dynamicStateCount = 0;
		}
		previousSize = size;

		if (light->type == LIGHT_TYPE_SPOT)
		{
			const VkRenderPassBeginInfo beginInfo = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.renderPass = shadowMapRenderPass,
				.framebuffer = ListGetPointer(shadowMapFramebuffers, framebufferIndex),
				.renderArea.extent = extent,
				.clearValueCount = 1,
				.pClearValues = &depthClearValue,
			};
			vkCmdBeginRenderPass(vkCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VulkanTestReturnResult(DrawMap(frustumIndex,
										   &pipelineBindInfo,
										   false,
										   pipelines.shadowMaps.opaqueMap,
										   pipelines.shadowMaps.map),
								   "Failed to draw map!");
			VulkanTestReturnResult(DrawActors(frustumIndex,
											  &pipelineBindInfo,
											  pipelines.shadowMaps.modelActors,
											  pipelines.shadowMaps.wallActors),
								   "Failed to draw actors!");

			vkCmdEndRenderPass(vkCommandBuffer);

			framebufferIndex++;
			frustumIndex++;
			continue;
		}
		if (light->type == LIGHT_TYPE_DIRECTIONAL)
		{
			for (shadowMapPushConstants.cascadeIndex = 0; shadowMapPushConstants.cascadeIndex < 4;
				 shadowMapPushConstants.cascadeIndex++)
			{
				const VkRenderPassBeginInfo beginInfo = {
					.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.renderPass = shadowMapRenderPass,
					.framebuffer = ListGetPointer(shadowMapFramebuffers, framebufferIndex),
					.renderArea.extent = extent,
					.clearValueCount = 1,
					.pClearValues = &depthClearValue,
				};
				vkCmdBeginRenderPass(vkCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

				VulkanTestReturnResult(DrawMap(shadowMapPushConstants.cascadeIndex + 1,
											   &pipelineBindInfo,
											   false,
											   pipelines.directionalLightShadowMaps.mapFrontFaces,
											   pipelines.directionalLightShadowMaps.mapFrontFaces),
									   "Failed to draw map front faces!");
				VulkanTestReturnResult(DrawMap(shadowMapPushConstants.cascadeIndex + 1,
											   &pipelineBindInfo,
											   false,
											   pipelines.directionalLightShadowMaps.mapBackFaces,
											   pipelines.directionalLightShadowMaps.mapBackFaces),
									   "Failed to draw map back faces!");
				VulkanTestReturnResult(DrawActors(shadowMapPushConstants.cascadeIndex + 1,
												  &pipelineBindInfo,
												  pipelines.directionalLightShadowMaps.modelActors,
												  pipelines.directionalLightShadowMaps.wallActors),
									   "Failed to draw actors!");

				vkCmdEndRenderPass(vkCommandBuffer);

				framebufferIndex++;
			}
			continue;
		}
		if (light->type == LIGHT_TYPE_POINT)
		{
			for (shadowMapPushConstants.faceIndex = 0; shadowMapPushConstants.faceIndex < 6;
				 shadowMapPushConstants.faceIndex++)
			{
				const VkRenderPassBeginInfo beginInfo = {
					.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.renderPass = shadowMapRenderPass,
					.framebuffer = ListGetPointer(shadowMapFramebuffers, framebufferIndex),
					.renderArea.extent = extent,
					.clearValueCount = 1,
					.pClearValues = &depthClearValue,
				};
				vkCmdBeginRenderPass(vkCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

				VulkanTestReturnResult(DrawMap(frustumIndex,
											   &pipelineBindInfo,
											   false,
											   pipelines.shadowMaps.opaqueMap,
											   pipelines.shadowMaps.map),
									   "Failed to draw map!");
				VulkanTestReturnResult(DrawActors(frustumIndex,
												  &pipelineBindInfo,
												  pipelines.shadowMaps.modelActors,
												  pipelines.shadowMaps.wallActors),
									   "Failed to draw actors!");

				vkCmdEndRenderPass(vkCommandBuffer);

				framebufferIndex++;
				frustumIndex++;
			}
		}
	}

	return VK_SUCCESS;
}

static inline VkResult UpdateGlobalLightingUniform(const Map *map)
{
	const GlobalLightingUniform globalLightingUniform = {
		.color = map->lightColor,
		.exposure = map->exposure,
	};
	const LunaBufferWriteInfo lightingBufferWriteInfo = {
		.bytes = sizeof(GlobalLightingUniform),
		.data = &globalLightingUniform,
		.stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.uniforms.lighting,
												 &lightingBufferWriteInfo),
						   "Failed to update lighting data!");

	return VK_SUCCESS;
}

static inline VkResult UpdateFogUniform(const Map *map)
{
	FogUniform fog = {
		.color = map->fogColor,
		.start = map->fogStart,
		.end = map->fogEnd,
	};
	const LunaBufferWriteInfo fogBufferWriteInfo = {
		.bytes = sizeof(fog),
		.data = &fog,
		.stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.uniforms.fog, &fogBufferWriteInfo),
						   "Failed to update fog data!");

	return VK_SUCCESS;
}

static inline VkResult HandleMapChangeFlags(Map *map)
{
	static_assert(MAP_LIGHT_CHANGED == MAP_EXPOSURE_CHANGED);
	if ((map->changeFlags & MAP_LIGHT_CHANGED) == MAP_LIGHT_CHANGED)
	{
		VulkanTestReturnResult(UpdateGlobalLightingUniform(map), "Failed to update global lighting uniform!");
	}

	if ((map->changeFlags & MAP_FOG_CHANGED) == MAP_FOG_CHANGED)
	{
		VulkanTestReturnResult(UpdateFogUniform(map), "Failed to update fog uniform!");
	}

	if ((map->changeFlags & MAP_VIEWMODEL_CHANGED) == MAP_VIEWMODEL_CHANGED)
	{
		VulkanTestReturnResult(LoadViewmodel(&map->viewmodel), "Failed to updated load viewmodel!");
	}

	map->changeFlags = 0;

	return VK_SUCCESS;
}

static inline bool HandleRendererQueuedActions()
{
	if (rendererQueuedActions == 0)
	{
		return true;
	}
	VulkanTest(lunaDeviceWaitIdle(device), "Failed to wait for device idle!");
	if (rendererQueuedActions & QUEUED_ACTION_CLEAR_ALL_MODELS)
	{
		if (!ClearModelCache())
		{
			return false;
		}
		buffers.player.modelDefinition = LoadModel(MODEL("player"));
		rendererQueuedActions &= ~QUEUED_ACTION_CLEAR_ALL_MODELS;
		if (GetMap() != NULL)
		{
			GetMap()->changeFlags |= MAP_VIEWMODEL_CHANGED;
		}
	}
	if (rendererQueuedActions & QUEUED_ACTION_CLEAR_ALL_TEXTURES)
	{
		if (!ClearTextureCache())
		{
			return false;
		}
		if (GetMap() != NULL)
		{
			VulkanTest(UpdateMapInstanceData(loadedMap, true, &buffers.opaqueMap),
					   "Failed to update opaque map instance data when reloading textures!");
			VulkanTest(UpdateMapInstanceData(loadedMap, false, &buffers.map),
					   "Failed to update map instance data when reloading textures!");
			if (loadedMap->renderSky)
			{
				skyTextureIndex = TextureIndex(loadedMap->skyTexture);
			}
			GetMap()->changeFlags |= MAP_VIEWMODEL_CHANGED;
		}
		rendererQueuedActions &= ~QUEUED_ACTION_CLEAR_ALL_TEXTURES;
	}
	if (rendererQueuedActions & QUEUED_ACTION_UPDATE_SHADOW_MAP_RESOLUTION)
	{
		if (GetState()->options.shadowMapQuality != SHADOW_MAP_RESOLUTION_DISABLED && GetMap() != NULL)
		{
			if (shadowMapRenderPass == VK_NULL_HANDLE || frustumCount == 1)
			{
				assert(shadowMapRenderPass == VK_NULL_HANDLE && frustumCount == 1);
				VulkanTest(LoadLights(loadedMap), "Failed to load lights into buffer!");
				VulkanTest(CreateMapModelDrawInfos(true,
												   &buffers.opaqueMap,
												   lunaGetBufferSize(buffers.opaqueMap.unculledShadedDrawInfo),
												   lunaGetBufferSize(buffers.opaqueMap.unculledUnshadedDrawInfo)),
						   "Failed to create opaque map model draw infos!");
				VulkanTest(CreateMapModelDrawInfos(false,
												   &buffers.map,
												   lunaGetBufferSize(buffers.map.unculledShadedDrawInfo),
												   lunaGetBufferSize(buffers.map.unculledUnshadedDrawInfo)),
						   "Failed to create map model draw infos!");
			}
			VulkanTest(CreateShadowMaps(loadedMap), "Failed to create shadow maps!");
		} else
		{
			if (lunaGetBufferSize(buffers.uniforms.lights) != 0 || frustumCount != 1)
			{
				VulkanTest(LoadLights(NULL), "Failed to load lights into buffer!");
				VulkanTest(CreateMapModelDrawInfos(true,
												   &buffers.opaqueMap,
												   lunaGetBufferSize(buffers.opaqueMap.unculledShadedDrawInfo),
												   lunaGetBufferSize(buffers.opaqueMap.unculledUnshadedDrawInfo)),
						   "Failed to create opaque map model draw infos!");
				VulkanTest(CreateMapModelDrawInfos(false,
												   &buffers.map,
												   lunaGetBufferSize(buffers.map.unculledShadedDrawInfo),
												   lunaGetBufferSize(buffers.map.unculledUnshadedDrawInfo)),
						   "Failed to create map model draw infos!");
			}
			if (shadowMapRenderPass != VK_NULL_HANDLE)
			{
				VulkanTest(CreateShadowMapRenderPass(NULL), "Failed to clean up shadow maps!");
			}
		}
		rendererQueuedActions &= ~QUEUED_ACTION_UPDATE_SHADOW_MAP_RESOLUTION;
	}
	if (rendererQueuedActions & QUEUED_ACTION_RELOAD_ALL_SHADERS)
	{
		lunaDestroyComputePipeline(device, pipelines.culling);
		lunaDestroyGraphicsPipeline(device, pipelines.ui);
		lunaDestroyGraphicsPipeline(device, pipelines.shadedMap);
		lunaDestroyGraphicsPipeline(device, pipelines.unshadedMap);
		lunaDestroyGraphicsPipeline(device, pipelines.sky);
		lunaDestroyGraphicsPipeline(device, pipelines.shadedModel);
		lunaDestroyGraphicsPipeline(device, pipelines.unshadedModel);
		lunaDestroyGraphicsPipeline(device, pipelines.shadedActorModel);
		lunaDestroyGraphicsPipeline(device, pipelines.unshadedActorModel);
		lunaDestroyGraphicsPipeline(device, pipelines.shadedActorWall);
		lunaDestroyGraphicsPipeline(device, pipelines.unshadedActorWall);

		if (!CreateCullingPipeline())
		{
			return false;
		}
		if (!CreateGraphicsPipelines())
		{
			return false;
		}
		VulkanTest(CreateDepthGraphicsPipelines(), "Failed to create shadow map graphics pipelines");

		rendererQueuedActions &= ~QUEUED_ACTION_RELOAD_ALL_SHADERS;
	}

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
		CreateRenderPass() && CreateDescriptorSetLayouts() && CreateCullingPipeline() && CreateGraphicsPipelines() &&
		CreateCullingDataClearPipeline() && CreateTextureSamplers() && CreateDescriptorSet() && CreateBuffers())
	{
		WriteDescriptorSet();

		// clang-format on
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

void VK_DPrintDevice()
{
	const char *gpuType = "Unknown";
	switch (physicalDeviceProperties.deviceType)
	{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			gpuType = "Integrated";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			gpuType = "Discrete";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			gpuType = "Virtual";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			gpuType = "CPU";
			break;
		default:
			break;
	}

	DPrintF("GPU: %s (%s)", COLOR_WHITE, physicalDeviceProperties.deviceName, gpuType);
	DPrintF("Vulkan: %u.%u.%u",
			COLOR_WHITE,
			VK_API_VERSION_MAJOR(physicalDeviceProperties.apiVersion),
			VK_API_VERSION_MINOR(physicalDeviceProperties.apiVersion),
			VK_API_VERSION_PATCH(physicalDeviceProperties.apiVersion));
}

void VK_DPrintSceneStatistics()
{
	DPrintF("Vulkan scene statistics:", COLOR_WHITE);
	DPrintF("Map vertex/index count: %u/%u",
			COLOR_WHITE,
			lunaGetBufferSize(buffers.map.vertices) / sizeof(ModelVertex),
			lunaGetBufferSize(buffers.map.indices) / sizeof(uint32_t));
	DPrintF("Actor model vertex/index count: %u/%u",
			COLOR_WHITE,
			lunaGetBufferSize(buffers.actorModels.vertices) / sizeof(ModelVertex),
			lunaGetBufferSize(buffers.actorModels.indices) / sizeof(uint32_t));
	DPrintF("Actor wall shaded/unshaded count: %u/%u",
			COLOR_WHITE,
			buffers.actorWalls.shadedInstanceCount,
			buffers.actorWalls.unshadedInstanceCount);
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

	VulkanTestResizeSwapchain(lunaBeginFrame(device, commandBuffer), "Failed to begin frame!");

	renderPassStarted = false;
	buffers.ui.freeQuads = buffers.ui.allocatedQuads;
#ifdef JPH_DEBUG_RENDERER
	buffers.debugDrawLines.bytesUsed = 0;
	buffers.debugDrawTriangles.bytesUsed = 0;
#endif

	return true;
}

bool VK_RenderMap(Map *map, Camera *camera)
{
	if (map != loadedMap)
	{
		if (!VK_LoadMap(map))
		{
			return false;
		}
	}

	VulkanTest(HandleMapChangeFlags(map), "Failed to handle map change flags!");

	VulkanTest(UpdateCameraUniform(camera), "Failed to update transform matrix!");

	VulkanTest(UpdateActors(), "Failed to update actors!");

	VulkanTest(CullModels(), "Failed to cull models!");

	VulkanTest(UpdateShadowMaps(map), "Failed to update shadow maps!");

	VulkanTest(UpdateViewModelMatrix(&map->viewmodel), "Failed to update viewmodel transform matrix!");

	const VkExtent2D extent = lunaGetSwapchainExtent();
	const LunaRenderPassBeginInfo beginInfo = {
		.renderArea.extent = extent,
		.depthAttachmentClearValue.depthStencil.depth = 0,
	};
	VulkanTest(lunaBeginRenderPass(device, commandBuffer, renderPass, &beginInfo), "Failed to begin render pass!");
	renderPassStarted = true;

	const VkViewport viewport = {
		.width = (float)extent.width,
		.height = (float)extent.height,
		.maxDepth = 1,
	};
	const LunaViewportBindInfo viewportBindInfo = {
		.viewportCount = 1,
		.viewports = &viewport,
	};
	const VkRect2D scissor = {
		.extent = extent,
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
	const LunaDescriptorSet descriptorSetHandles[] = {
		descriptorSets.common.set,
		descriptorSets.spotLightShadowMaps.set,
		descriptorSets.pointLightShadowMaps.set,
	};
	const LunaGraphicsPipelineBindInfo pipelineBindInfo = {
		.descriptorSetBindInfo.descriptorSetCount = 3,
		.descriptorSetBindInfo.descriptorSets = descriptorSetHandles,
		.dynamicStateCount = sizeof(dynamicStateBindInfos) / sizeof(*dynamicStateBindInfos),
		.dynamicStates = dynamicStateBindInfos,
	};

	const LunaGraphicsPipelineBindInfo prepassPipelineBindInfo = {
		.descriptorSetBindInfo.descriptorSetCount = 1,
		.descriptorSetBindInfo.descriptorSets = &descriptorSets.common.set,
		.dynamicStateCount = sizeof(dynamicStateBindInfos) / sizeof(*dynamicStateBindInfos),
		.dynamicStates = dynamicStateBindInfos,
	};
	VulkanTest(DrawMap(0, &prepassPipelineBindInfo, true, LUNA_NULL_HANDLE, LUNA_NULL_HANDLE),
			   "Failed to draw map depth prepass!");
	VulkanTest(DrawActors(0,
						  &prepassPipelineBindInfo,
						  pipelines.depthPrepass.modelActors,
						  pipelines.depthPrepass.wallActors),
			   "Failed to draw actors depth prepass!");
	lunaNextSubpass(commandBuffer);

	if (map->renderSky)
	{
		VulkanTest(DrawSky(&pipelineBindInfo), "Failed to draw sky!");
	}
	VulkanTest(DrawMap(0, &pipelineBindInfo, false, LUNA_NULL_HANDLE, LUNA_NULL_HANDLE), "Failed to draw map!");
	VulkanTest(DrawActors(0, &pipelineBindInfo, LUNA_NULL_HANDLE, LUNA_NULL_HANDLE), "Failed to draw actors!");
	VulkanTest(DrawDebugRenderer(&pipelineBindInfo), "Failed to draw Jolt debug renderer!");
	if (camera->showPlayerModel)
	{
		VulkanTest(DrawPlayer(map, &pipelineBindInfo), "Failed to draw player!");
	}
	// Viewmodel must be drawn last
	if (map->viewmodel.enabled && camera == &map->player.playerCamera)
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

	const VkExtent2D extent = lunaGetSwapchainExtent();

	if (!renderPassStarted)
	{
		const LunaRenderPassBeginInfo beginInfo = {
			.renderArea.extent = extent,
			.depthAttachmentClearValue.depthStencil.depth = 1,
		};
		VulkanTest(lunaBeginRenderPass(device, commandBuffer, renderPass, &beginInfo), "Failed to begin render pass!");
		lunaNextSubpass(commandBuffer); // Skip the depth prepass since we aren't using depth anyway
	}

	if (buffers.ui.freeQuads != buffers.ui.allocatedQuads)
	{
		const VkViewport viewport = {
			.width = (float)extent.width,
			.height = (float)extent.height,
			.maxDepth = 1,
		};
		const LunaViewportBindInfo viewportBindInfo = {
			.viewportCount = 1,
			.viewports = &viewport,
		};
		const VkRect2D scissor = {
			.extent = extent,
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
		const LunaDescriptorSet descriptorSetHandles[] = {
			descriptorSets.common.set,
			descriptorSets.spotLightShadowMaps.set,
			descriptorSets.pointLightShadowMaps.set,
		};
		const LunaGraphicsPipelineBindInfo pipelineBindInfo = {
			.descriptorSetBindInfo.descriptorSetCount = 3,
			.descriptorSetBindInfo.descriptorSets = descriptorSetHandles,
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

	const LunaPresentInfo presentInfo = {};
	const VkPipelineStageFlags2 waitStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
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
	VulkanTestInternal(lunaDeviceWaitIdle(device), (void)0, "Failed to wait for device idle!");
	free(buffers.ui.vertexData);
	free(buffers.ui.indexData);
	free(buffers.player.instanceData);
	VulkanTestInternal(CreateShadowMapRenderPass(NULL), (void)0, "Failed to destroy shadow map!");
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
		lunaDestroyImage(device, lightmap);
		lightmap = LUNA_NULL_HANDLE;

		loadedMap = NULL;

		return true;
	}

	VulkanTest(LoadLightmap(map), "Failed to load lightmap!");

	VulkanTest(LoadLights(map), "Failed to load lights into buffer!");
	VulkanTest(CreateShadowMaps(map), "Failed to create shadow maps!")

			VulkanTest(LoadMapModelsToBuffer(map->modelCount, map->models, true, &buffers.opaqueMap),
					   "Failed to load opaque map models!");
	VulkanTest(LoadMapModelsToBuffer(map->modelCount, map->models, false, &buffers.map), "Failed to load map models!");

	VulkanTest(LoadViewmodel(&map->viewmodel), "Failed to load viewmodel!");

	VulkanTest(LoadActors(&map->actors), "Failed to load actors!");

	if (map->renderSky)
	{
		VulkanTest(LoadSky(LoadModel(MODEL("sky"))), "Failed to load sky model!");
		skyTextureIndex = TextureIndex(map->skyTexture);
	}

	VulkanTest(UpdateGlobalLightingUniform(map), "Failed to update global lighting uniform!");

	VulkanTest(UpdateFogUniform(map), "Failed to update fog uniform!");

	loadedMap = map;

	return true;
}

bool VK_UpdateViewportSize()
{
	VulkanTest(lunaDeviceWaitIdle(device), "Failed to wait for device to become idle!");
	const Vector2 windowSize = ActualWindowSizeIgnoreDPI();
	const LunaSwapchainResizeInfo swapchainResizeInfo = {
		.newSize.width = (uint32_t)windowSize.x,
		.newSize.height = (uint32_t)windowSize.y,
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

void VK_DrawLine(const Vector2 start, const Vector2 end, const float thickness, const Color color)
{
	const float dx = end.x - start.x;
	const float dy = end.y - start.y;
	const float distance = 2.0f * sqrtf(dx * dx + dy * dy);

	const mat4 matrix = {
		{
			VK_X_TO_NDC(-thickness * dy / distance + start.x),
			VK_Y_TO_NDC(thickness * dx / distance + start.y),
			0,
			0,
		},
		{
			VK_X_TO_NDC(-thickness * dy / distance + end.x),
			VK_Y_TO_NDC(thickness * dx / distance + end.y),
			0,
			0,
		},
		{
			VK_X_TO_NDC(thickness * dy / distance + end.x),
			VK_Y_TO_NDC(-thickness * dx / distance + end.y),
			0,
			0,
		},
		{
			VK_X_TO_NDC(thickness * dy / distance + start.x),
			VK_Y_TO_NDC(-thickness * dx / distance + start.y),
			0,
			0,
		},
	};
	DrawQuadInternal(matrix, &color, -1);
}

void VK_DrawRectOutline(const Vector2 position, const Vector2 size, const float thickness, const Color color)
{
	const float halfThickness = thickness / 2.0f;
	const float left = position.x + halfThickness;
	const float right = position.x + size.x;
	const float top = position.y - halfThickness;
	const float bottom = position.y + size.y;
	const float horizontalRight = right - halfThickness;
	const float verticalBottom = position.y + size.y + halfThickness;
	VK_DrawLine(v2(left, position.y), v2(horizontalRight, position.y), thickness, color);
	VK_DrawLine(v2(left, bottom), v2(horizontalRight, bottom), thickness, color);
	VK_DrawLine(v2(position.x, top), v2(position.x, verticalBottom), thickness, color);
	VK_DrawLine(v2(right, top), v2(right, verticalBottom), thickness, color);
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
	if (buffers.debugDrawLines.allocatedSize < buffers.debugDrawLines.bytesUsed + sizeof(DebugDrawVertex) * 2)
	{
		buffers.debugDrawLines.allocatedSize += sizeof(DebugDrawVertex) * 2 * 16;

		DebugDrawVertex *newVertices = realloc(buffers.debugDrawLines.vertices, buffers.debugDrawLines.allocatedSize);
		CheckAlloc(newVertices);
		buffers.debugDrawLines.vertices = newVertices;
	}

	const float a = 1;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	DebugDrawVertex *bufferVertices = buffers.debugDrawLines.vertices + buffers.debugDrawLines.bytesUsed;

	bufferVertices[0] = (DebugDrawVertex){
		.position = *from,
		.color = {r, g, b, a},
	};
	bufferVertices[1] = (DebugDrawVertex){
		.position = *to,
		.color = {r, g, b, a},
	};

	buffers.debugDrawLines.bytesUsed += 2 * sizeof(DebugDrawVertex);
#else
	(void)from;
	(void)to;
	(void)color;
#endif
}

void VK_DrawJoltDebugRendererTriangle(const Vector3 *vertices, const uint32_t color)
{
#ifdef JPH_DEBUG_RENDERER
	if (buffers.debugDrawTriangles.allocatedSize < buffers.debugDrawTriangles.bytesUsed + sizeof(DebugDrawVertex) * 3)
	{
		buffers.debugDrawTriangles.allocatedSize += sizeof(DebugDrawVertex) * 3 * 16;

		DebugDrawVertex *newVertices = realloc(buffers.debugDrawTriangles.vertices,
											   buffers.debugDrawTriangles.allocatedSize);
		CheckAlloc(newVertices);
		buffers.debugDrawTriangles.vertices = newVertices;
	}

	const float a = 0.5f;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	DebugDrawVertex *bufferVertices = buffers.debugDrawTriangles.vertices + buffers.debugDrawTriangles.bytesUsed;

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

	buffers.debugDrawTriangles.bytesUsed += 3 * sizeof(DebugDrawVertex);
#else
	(void)vertices;
	(void)color;
#endif
}
