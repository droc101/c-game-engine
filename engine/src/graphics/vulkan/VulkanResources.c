//
// Created by Noah on 12/18/2024.
//

#include <cglm/cglm.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanResources.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <luna/luna.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaImage.h>
#include <luna/lunaTypes.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

static inline VkResult CreateModelBuffer(ModelBuffer *buffer, const char *usage)
{
	const LunaBufferCreationInfo vertexBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	const LunaBufferCreationInfo indexBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	const LunaBufferCreationInfo storageBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	const LunaBufferCreationInfo drawInfoBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &vertexBufferCreationInfo, &buffer->vertices),
						   "Failed to create shaded %s vertex buffer!",
						   usage);
	VulkanTestReturnResult(lunaCreateBuffer(device, &indexBufferCreationInfo, &buffer->indices),
						   "Failed to create shaded %s index buffer!",
						   usage);
	if (buffer == &buffers.actorModels)
	{
		VulkanTestReturnResult(lunaCreateBuffer(device, &storageBufferCreationInfo, &buffer->instanceData),
							   "Failed to create shaded %s instance data buffer!",
							   usage);
		const LunaBufferCreationInfo instanceIndicesBufferCreationInfo = {
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			.queueFamilyIndexCount = 1,
			.queueFamilyIndices = &queueFamilyIndex,
		};
		VulkanTestReturnResult(lunaCreateBuffer(device,
												&instanceIndicesBufferCreationInfo,
												&buffer->shadedInstanceIndices),
							   "Failed to create shaded %s instance indices buffer!",
							   usage);
		VulkanTestReturnResult(lunaCreateBuffer(device,
												&instanceIndicesBufferCreationInfo,
												&buffer->unshadedInstanceIndices),
							   "Failed to create unshaded %s instance indices buffer!",
							   usage);
	} else
	{
		VulkanTestReturnResult(lunaCreateBuffer(device, &vertexBufferCreationInfo, &buffer->instanceData),
							   "Failed to create shaded %s instance data buffer!",
							   usage);
		VulkanTestReturnResult(lunaCreateBuffer(device, &drawInfoBufferCreationInfo, &buffer->unculledShadedDrawInfo),
							   "Failed to create shaded %s unculled draw info buffer!",
							   usage);
		VulkanTestReturnResult(lunaCreateBuffer(device, &drawInfoBufferCreationInfo, &buffer->unculledUnshadedDrawInfo),
							   "Failed to create unshaded %s unculled draw info buffer!",
							   usage);
	}
	VulkanTestReturnResult(lunaCreateBuffer(device, &drawInfoBufferCreationInfo, &buffer->shadedDrawInfo),
						   "Failed to create shaded %s draw info buffer!",
						   usage);
	VulkanTestReturnResult(lunaCreateBuffer(device, &drawInfoBufferCreationInfo, &buffer->unshadedDrawInfo),
						   "Failed to create unshaded %s draw info buffer!",
						   usage);
	VulkanTestReturnResult(lunaCreateBuffer(device, &storageBufferCreationInfo, &buffer->shadedCullingInfo),
						   "Failed to create %s shaded materials culling info buffer!",
						   usage);
	VulkanTestReturnResult(lunaCreateBuffer(device, &storageBufferCreationInfo, &buffer->unshadedCullingInfo),
						   "Failed to create %s unshaded materials culling info buffer!",
						   usage);
	return VK_SUCCESS;
}

static inline VkResult CreateUiBuffers()
{
	static const uint32_t MAX_UI_QUADS_INIT = 8192;

	buffers.ui.allocatedQuads = 0;
	buffers.ui.freeQuads = MAX_UI_QUADS_INIT;

	const size_t vertexBufferAllocationSize = MAX_UI_QUADS_INIT * 4 * sizeof(UiVertex);
	const LunaBufferCreationInfo vertexBufferCreationInfo = {
		.size = vertexBufferAllocationSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &vertexBufferCreationInfo, &buffers.ui.vertexBuffer),
						   "Failed to create UI vertex buffer!");
	buffers.ui.vertexData = malloc(vertexBufferAllocationSize);
	CheckAlloc(buffers.ui.vertexData);

	const size_t indexBufferAllocationSize = MAX_UI_QUADS_INIT * 6 * sizeof(uint32_t);
	const LunaBufferCreationInfo indexBufferCreationInfo = {
		.size = indexBufferAllocationSize,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &indexBufferCreationInfo, &buffers.ui.indexBuffer),
						   "Failed to create UI index buffer!");
	buffers.ui.indexData = malloc(indexBufferAllocationSize);
	CheckAlloc(buffers.ui.indexData);

	return VK_SUCCESS;
}

static inline VkResult CreateUniformBuffers()
{
	const VkDeviceSize alignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
	const LunaBufferCreationInfo cameraUniformBufferCreationInfo = {
		.size = sizeof(CameraUniform),
		.alignment = alignment,
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &cameraUniformBufferCreationInfo, &buffers.uniforms.camera),
						   "Failed to create camera uniform buffer!");
	const LunaBufferCreationInfo lightingBufferCreationInfo = {
		.size = sizeof(GlobalLightingUniform),
		.alignment = alignment,
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &lightingBufferCreationInfo, &buffers.uniforms.lighting),
						   "Failed to create lighting uniform buffer!");
	const LunaBufferCreationInfo fogBufferCreationInfo = {
		.size = sizeof(FogUniform),
		.alignment = alignment,
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &fogBufferCreationInfo, &buffers.uniforms.fog),
						   "Failed to create fog uniform buffer!");
	const LunaBufferCreationInfo lightsBufferCreationInfo = {
		.alignment = physicalDeviceProperties.limits.minStorageBufferOffsetAlignment,
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &lightsBufferCreationInfo, &buffers.uniforms.lights),
						   "Failed to create lights buffer!");

	return VK_SUCCESS;
}

static inline VkResult CreateFrustumsBuffer()
{
	const LunaBufferCreationInfo creationInfo = {
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &creationInfo, &buffers.frustums),
						   "Failed to create frustums buffer!");

	return VK_SUCCESS;
}

static inline VkResult CreateSkyBuffers()
{
	static const size_t SKY_MAX_VERTICES_INIT = 559;
	static const size_t SKY_MAX_INDICES_INIT = 2880;

	const LunaBufferCreationInfo verticesBufferCreationInfo = {
		.size = sizeof(SkyVertex) * SKY_MAX_VERTICES_INIT,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &verticesBufferCreationInfo, &buffers.sky.vertices),
						   "Failed to create sky vertex buffer!");
	const LunaBufferCreationInfo indicesBufferCreationInfo = {
		.size = sizeof(uint32_t) * SKY_MAX_INDICES_INIT,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &indicesBufferCreationInfo, &buffers.sky.indices),
						   "Failed to create sky index buffer!");

	return VK_SUCCESS;
}

static inline VkResult CreateActorWallBuffers()
{
	const ActorWallVertex vertices[12] = {
		{
			.position.x = 0.5f,
			.position.y = -0.5f,
			.uv.x = 1,
			.uv.y = 1,
		},
		{
			.position.x = 0.5f,
			.position.y = 0.5f,
			.uv.x = 1,
		},
		{
			.position.x = -0.5f,
			.position.y = 0.5f,
		},
		{
			.position.x = -0.5f,
			.position.y = -0.5f,
			.uv.y = 1,
		},
		{
			.position.x = 0.5f,
			.position.y = -0.5f,
			.uv.x = 1,
			.uv.y = 1,
		},
		{
			.position.x = -0.5f,
			.position.y = 0.5f,
		},

		{
			.position.x = -0.5f,
			.position.y = 0.5f,
			.uv.x = 1,
		},
		{
			.position.x = 0.5f,
			.position.y = 0.5f,
		},
		{
			.position.x = 0.5f,
			.position.y = -0.5f,
			.uv.y = 1,
		},
		{
			.position.x = -0.5f,
			.position.y = 0.5f,
			.uv.x = 1,
		},
		{
			.position.x = 0.5f,
			.position.y = -0.5f,
			.uv.y = 1,
		},
		{
			.position.x = -0.5f,
			.position.y = -0.5f,
			.uv.x = 1,
			.uv.y = 1,
		},
	};
	const LunaBufferWriteInfo vertexDataWriteInfo = {
		.bytes = sizeof(vertices),
		.data = vertices,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	};

	const LunaBufferCreationInfo verticesBufferCreationInfo = {
		.size = sizeof(ActorWallVertex) * 12,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &verticesBufferCreationInfo, &buffers.actorWalls.vertices),
						   "Failed to create actor walls vertex buffer!");
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.actorWalls.vertices,
												 &vertexDataWriteInfo),
						   "Failed to write actor vertex data to buffer!");


	const LunaBufferCreationInfo drawInfoBufferCreationInfo = {
		.size = sizeof(VkDrawIndirectCommand),
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &drawInfoBufferCreationInfo, &buffers.actorWalls.shadedDrawInfo),
						   "Failed to create shaded actor walls draw info buffer!");
	VulkanTestReturnResult(lunaCreateBuffer(device, &drawInfoBufferCreationInfo, &buffers.actorWalls.unshadedDrawInfo),
						   "Failed to create unshaded actor walls draw info buffer!");

	const LunaDescriptorBufferInfo drawInfoBuffers[] = {
		{
			.buffer = buffers.actorWalls.shadedDrawInfo,
		},
		{
			.buffer = buffers.actorWalls.unshadedDrawInfo,
		},
	};
	const LunaWriteDescriptorSet drawInfoWrite = {
		.descriptorSet = descriptorSets.culling.set,
		.bindingName = "Output Draw Info",
		.descriptorArrayElement = 4,
		.descriptorCount = 2,
		.bufferInfos = drawInfoBuffers,
	};
	lunaWriteDescriptorSets(device, 1, &drawInfoWrite);

	const LunaBufferCreationInfo vertexBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	const LunaBufferCreationInfo storageBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device,
											&vertexBufferCreationInfo,
											&buffers.actorWalls.shadedInstanceIndices),
						   "Failed to create shaded actor walls instance indices buffer!");
	VulkanTestReturnResult(lunaCreateBuffer(device,
											&vertexBufferCreationInfo,
											&buffers.actorWalls.unshadedInstanceIndices),
						   "Failed to create unshaded actor walls instance indices buffer!");
	VulkanTestReturnResult(lunaCreateBuffer(device, &storageBufferCreationInfo, &buffers.actorWalls.instanceData),
						   "Failed to create actor walls instance data buffer!");
	VulkanTestReturnResult(lunaCreateBuffer(device, &storageBufferCreationInfo, &buffers.actorWalls.shadedCullingInfo),
						   "Failed to create shaded actor walls culling info buffer!");
	VulkanTestReturnResult(lunaCreateBuffer(device,
											&storageBufferCreationInfo,
											&buffers.actorWalls.unshadedCullingInfo),
						   "Failed to create unshaded actor walls culling info buffer!");

	return VK_SUCCESS;
}

// TODO: Only skin 0 of LOD 0 is ever loaded and used currently
static inline VkResult CreatePlayerBuffers()
{
	buffers.player.modelDefinition = LoadModel(MODEL("player"));
	const ModelDefinition *model = buffers.player.modelDefinition;
	const ModelLod *lod = model->lods;

	const VkDeviceSize vertexBufferSize = sizeof(ModelVertex) * lod->vertexCount;
	const LunaBufferCreationInfo vertexBufferCreationInfo = {
		.size = vertexBufferSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &vertexBufferCreationInfo, &buffers.player.buffers.vertices),
						   "Failed to create player model vertex buffer!");
	const LunaBufferWriteInfo vertexDataWriteInfo = {
		.bytes = vertexBufferSize,
		.data = lod->vertexData,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.player.buffers.vertices,
												 &vertexDataWriteInfo),
						   "Failed to write player vertex data to buffer!");

	const VkDeviceSize indexBufferSize = sizeof(uint32_t) * lod->totalIndexCount;
	const LunaBufferCreationInfo indexBufferCreationInfo = {
		.size = indexBufferSize,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &indexBufferCreationInfo, &buffers.player.buffers.indices),
						   "Failed to create player model index buffer!");

	const LunaBufferCreationInfo instanceDataBufferCreationInfo = {
		.size = sizeof(ModelInstanceData) * model->materialSlotCount,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device,
											&instanceDataBufferCreationInfo,
											&buffers.player.buffers.instanceData),
						   "Failed to create player model instance data buffer!");

	VkDeviceSize indexOffset = 0;
	uint32_t shadedDrawCount = 0;
	uint32_t unshadedDrawCount = 0;
	uint32_t *indices = malloc(sizeof(uint32_t) * lod->totalIndexCount);
	CheckAlloc(indices);
	buffers.player.instanceData = malloc(sizeof(ModelInstanceData) * model->materialSlotCount);
	CheckAlloc(buffers.player.instanceData);
	VkDrawIndexedIndirectCommand *shadedDrawInfo = malloc(sizeof(VkDrawIndexedIndirectCommand) *
														  model->materialSlotCount);
	CheckAlloc(shadedDrawInfo);
	VkDrawIndexedIndirectCommand *unshadedDrawInfo = malloc(sizeof(VkDrawIndexedIndirectCommand) *
															model->materialSlotCount);
	CheckAlloc(unshadedDrawInfo);
	for (uint32_t slotIndex = 0; slotIndex < model->materialSlotCount; slotIndex++)
	{
		const Material *material = &model->materials[model->skinMaterialIndices[0][slotIndex]];
		const ModelComponent *component = &lod->components[slotIndex];

		memcpy(indices + indexOffset, component->indices, component->indexCount * sizeof(uint32_t));
		indexOffset += component->indexCount;

		buffers.player.instanceData[slotIndex].materialColor = material->color;
		buffers.player.instanceData[slotIndex].textureIndex = TextureIndex(material->texture);

		if (material->shader == SHADER_SHADED)
		{
			shadedDrawInfo[shadedDrawCount].indexCount = component->indexCount;
			shadedDrawInfo[shadedDrawCount].instanceCount = 1;
			shadedDrawInfo[shadedDrawCount].firstIndex = 0;
			shadedDrawInfo[shadedDrawCount].vertexOffset = 0;
			shadedDrawInfo[shadedDrawCount].firstInstance = 0;
			shadedDrawCount++;
		} else if (material->shader == SHADER_UNSHADED)
		{
			unshadedDrawInfo[unshadedDrawCount].indexCount = component->indexCount;
			unshadedDrawInfo[unshadedDrawCount].instanceCount = 1;
			unshadedDrawInfo[unshadedDrawCount].firstIndex = 0;
			unshadedDrawInfo[unshadedDrawCount].vertexOffset = 0;
			unshadedDrawInfo[unshadedDrawCount].firstInstance = 0;
			unshadedDrawCount++;
		}
	}

	const LunaBufferWriteInfo indexDataWriteInfo = {
		.bytes = indexBufferSize,
		.data = indices,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.player.buffers.indices,
												 &indexDataWriteInfo),
						   "Failed to write player index data to buffer!");

	const VkDeviceSize shadedDrawInfoSize = sizeof(VkDrawIndexedIndirectCommand) * shadedDrawCount;
	const LunaBufferCreationInfo shadedDrawInfoBufferCreationInfo = {
		.size = shadedDrawInfoSize,
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device,
											&shadedDrawInfoBufferCreationInfo,
											&buffers.player.buffers.shadedDrawInfo),
						   "Failed to create player model shaded draw info buffer!");
	const LunaBufferWriteInfo shadedDrawInfoDataWriteInfo = {
		.bytes = shadedDrawInfoSize,
		.data = shadedDrawInfo,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.player.buffers.shadedDrawInfo,
												 &shadedDrawInfoDataWriteInfo),
						   "Failed to write player shaded draw info data to buffer!");

	const VkDeviceSize unshadedDrawInfoSize = sizeof(VkDrawIndexedIndirectCommand) * unshadedDrawCount;
	const LunaBufferCreationInfo unshadedDrawInfoBufferCreationInfo = {
		.size = unshadedDrawInfoSize,
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device,
											&unshadedDrawInfoBufferCreationInfo,
											&buffers.player.buffers.unshadedDrawInfo),
						   "Failed to create player model unshaded draw info buffer!");
	const LunaBufferWriteInfo unshadedDrawInfoDataWriteInfo = {
		.bytes = unshadedDrawInfoSize,
		.data = unshadedDrawInfo,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device,
												 commandBuffer,
												 buffers.player.buffers.unshadedDrawInfo,
												 &unshadedDrawInfoDataWriteInfo),
						   "Failed to write player unshaded draw info data to buffer!");

	free(indices);
	free(shadedDrawInfo);
	free(unshadedDrawInfo);

	return VK_SUCCESS;
}

static inline VkResult CreateDebugDrawBuffers()
{
#ifdef JPH_DEBUG_RENDERER
	const LunaBufferCreationInfo linesVertexBuffer = {
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &linesVertexBuffer, &buffers.debugDrawLines.buffer),
						   "Failed to create debug draw lines buffer!");

	const LunaBufferCreationInfo vertexVertexBuffer = {
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &vertexVertexBuffer, &buffers.debugDrawTriangles.buffer),
						   "Failed to create debug draw triangles buffer!");
#endif

	return VK_SUCCESS;
}

bool CreateBuffers()
{
	VulkanTest(CreateUiBuffers(), "Failed to create UI buffers!");
	VulkanTest(CreateUniformBuffers(), "Failed to create uniform buffers!");
	VulkanTest(CreateFrustumsBuffer(), "Failed to create frustums buffer!");
	VulkanTest(CreateModelBuffer(&buffers.actorModels, "actor models"), "Failed to create actor models buffers!");
	VulkanTest(CreateActorWallBuffers(), "Failed to create actor wall buffers!");
	VulkanTest(CreateModelBuffer(&buffers.opaqueMap, "opaque map"), "Failed to create opaque map buffers!");
	VulkanTest(CreateModelBuffer(&buffers.map, "map"), "Failed to create map buffers!");
	VulkanTest(CreateModelBuffer(&buffers.viewmodel, "viewmodel"), "Failed to create viewmodel buffers!");
	VulkanTest(CreateSkyBuffers(), "Failed to create sky buffers!");
	VulkanTest(CreatePlayerBuffers(), "Failed to create player buffers!");
	VulkanTest(CreateDebugDrawBuffers(), "Failed to create debug draw buffers!");

	return true;
}

VkResult ResizeDebugDrawBuffers()
{
#ifdef JPH_DEBUG_RENDERER
	VulkanTestReturnResult(lunaGrowBuffer(device,
										  commandBuffer,
										  &buffers.debugDrawLines.buffer,
										  buffers.debugDrawLines.allocatedSize),
						   "Failed to grow debug draw lines buffer!");
	VulkanTestReturnResult(lunaGrowBuffer(device,
										  commandBuffer,
										  &buffers.debugDrawTriangles.buffer,
										  buffers.debugDrawTriangles.allocatedSize),
						   "Failed to grow debug draw triangles buffer!");
#endif

	return VK_SUCCESS;
}

bool LoadTexture(const Image *image)
{
	const bool useMipmaps = GetState()->options.mipmaps && image->mipmaps;
	LunaSampler sampler = LUNA_NULL_HANDLE;
	if (image->filter && image->repeat)
	{
		sampler = useMipmaps ? textureSamplers.linearRepeatAnisotropy : textureSamplers.linearRepeatNoAnisotropy;
	}
	if (image->filter && !image->repeat)
	{
		sampler = useMipmaps ? textureSamplers.linearNoRepeatAnisotropy : textureSamplers.linearNoRepeatNoAnisotropy;
	}
	if (!image->filter && image->repeat)
	{
		sampler = textureSamplers.nearestRepeatNoAnisotropy;
	}
	if (!image->filter && !image->repeat)
	{
		sampler = textureSamplers.nearestNoRepeatNoAnisotropy;
	}

	const VkPipelineStageFlags2 waitStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	// TODO: Using the same semaphore for wait and signal slows down loading multiple textures per frame.
	//  The wait semaphore is important because this needs to wait for the previous render to be finished before it can
	//  actually start, otherwise it could cause the previous frame to have garbage data.
	//  For now I'm just using one semaphore for both image->render and for render->image.
	const LunaCommandBufferSubmitInfo submitInfo = {
		.queue = queue,
		.waitSemaphoreCount = 1,
		.waitSemaphores = &semaphore,
		.waitDstStageMasks = &waitStage,
		.signalSemaphoreCount = 1,
		.signalSemaphores = &semaphore,
	};
	const VkFormat format = image->pixelFormat == PIXEL_FORMAT_RGBA16F ? VK_FORMAT_R16G16B16A16_SFLOAT
																	   : VK_FORMAT_R8G8B8A8_UNORM;
	const size_t bytesPerTexelChannel = image->pixelFormat == PIXEL_FORMAT_RGBA16F ? sizeof(_Float16) : sizeof(uint8_t);
	const LunaImageCreationInfo imageCreationInfo = {
		.format = format,
		.width = image->width,
		.height = image->height,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.writeInfo.bytes = image->width * image->height * 4 * bytesPerTexelChannel,
		.writeInfo.pixels = image->pixelData,
		.writeInfo.mipmapLevels = useMipmaps ? (uint8_t)log2(max(image->width, image->height)) + 1 : 1,
		.writeInfo.generateMipmaps = useMipmaps,
		.writeInfo.mipmapFilter = VK_FILTER_LINEAR,
		.writeInfo.sourceStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		.writeInfo.destinationStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		.writeInfo.destinationAccessMask = VK_ACCESS_SHADER_READ_BIT,
		.writeInfo.submitInfo = &submitInfo,
		.sampler = sampler,
	};
	LunaImage lunaImage = LUNA_NULL_HANDLE;
	const size_t index = textures.length;
	VulkanTest(lunaCreateImage(device, secondaryCommandBuffer, &imageCreationInfo, &lunaImage),
			   "Failed to create texture!");
	imageAssetIdToIndexMap[image->id] = index;
	ListAdd(textures, lunaImage);

	const LunaDescriptorImageInfo imageInfo = {
		.image = lunaImage,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};
	const LunaWriteDescriptorSet writeDescriptor = {
		.descriptorSet = descriptorSets.common.set,
		.bindingName = "Textures",
		.descriptorArrayElement = index,
		.descriptorCount = 1,
		.imageInfos = &imageInfo,
	};
	lunaWriteDescriptorSets(device, 1, &writeDescriptor);

	return true;
}
