//
// Created by Noah on 12/18/2024.
//

#include <cglm/cglm.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanResources.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Options.h>
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

static const LunaBufferCreationInfo UNALIGNED_BUFFER_CREATION_INFO = {
	.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
};
static LunaBufferCreationInfo storageBufferCreationInfo = {
	.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
};

static inline void CreateModelBuffer(ModelBuffer *buffer, const char *usage)
{
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffer->instanceData),
			   "Failed to create %s instance data buffer!",
			   usage);
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffer->shadedDrawInfo),
			   "Failed to create shaded %s draw info buffer!",
			   usage);
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffer->unshadedDrawInfo),
			   "Failed to create unshaded %s draw info buffer!",
			   usage);
}

static inline void CreateUiBuffers()
{
	static const uint32_t MAX_UI_QUADS_INIT = 8192;

	buffers.ui.allocatedQuads = 0;
	buffers.ui.freeQuads = MAX_UI_QUADS_INIT;

	const size_t vertexBufferAllocationSize = MAX_UI_QUADS_INIT * 4 * sizeof(UiVertex);
	const LunaBufferCreationInfo vertexBufferCreationInfo = {
		.size = vertexBufferAllocationSize,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
	};
	VulkanTest(lunaCreateBuffer(device, &vertexBufferCreationInfo, &buffers.ui.vertexBuffer),
			   "Failed to create UI vertex buffer!");
	buffers.ui.vertexData = malloc(vertexBufferAllocationSize);
	CheckAlloc(buffers.ui.vertexData);

	const size_t indexBufferAllocationSize = MAX_UI_QUADS_INIT * 6 * sizeof(uint32_t);
	const LunaBufferCreationInfo indexBufferCreationInfo = {
		.size = indexBufferAllocationSize,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
	};
	VulkanTest(lunaCreateBuffer(device, &indexBufferCreationInfo, &buffers.ui.indexBuffer),
			   "Failed to create UI index buffer!");
	buffers.ui.indexData = malloc(indexBufferAllocationSize);
	CheckAlloc(buffers.ui.indexData);
}

static inline void CreateUniformBuffers()
{
	const LunaBufferCreationInfo cameraUniformBufferCreationInfo = {
		.size = sizeof(CameraUniform),
		.alignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
	};
	VulkanTest(lunaCreateBuffer(device, &cameraUniformBufferCreationInfo, &buffers.uniforms.camera),
			   "Failed to create camera uniform buffer!");
	const LunaBufferCreationInfo lightingBufferCreationInfo = {
		.size = sizeof(GlobalLightingUniform),
		.alignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
	};
	VulkanTest(lunaCreateBuffer(device, &lightingBufferCreationInfo, &buffers.uniforms.lighting),
			   "Failed to create lighting uniform buffer!");
	const LunaBufferCreationInfo fogBufferCreationInfo = {
		.size = sizeof(FogUniform),
		.alignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
	};
	VulkanTest(lunaCreateBuffer(device, &fogBufferCreationInfo, &buffers.uniforms.fog),
			   "Failed to create fog uniform buffer!");
	const LunaBufferCreationInfo lightsBufferCreationInfo = {
		.alignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
	};
	VulkanTest(lunaCreateBuffer(device, &lightsBufferCreationInfo, &buffers.uniforms.lights),
			   "Failed to create lights buffer!");
	const LunaBufferCreationInfo clustersBufferCreationInfo = {
		.size = 8 * 8 * 8 * sizeof(Cluster),
		.alignment = physicalDeviceProperties.limits.minStorageBufferOffsetAlignment,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
	};
	VulkanTest(lunaCreateBuffer(device, &clustersBufferCreationInfo, &buffers.uniforms.clusters),
			   "Failed to create clusters buffer!");

	const LunaBufferCreationInfo softShadowKernelsBufferCreationInfo = {
		.size = sizeof(vec2) * specializationConstants.sampleCount,
		.alignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
	};
	VulkanTest(lunaCreateBuffer(device, &softShadowKernelsBufferCreationInfo, &buffers.uniforms.softShadowKernels),
			   "Failed to create soft shadow kernels uniform buffer!");
	UpdateSoftShadowKernels();
}

static inline void CreateFrustumsBuffers()
{
	VulkanTest(lunaCreateBuffer(device, &storageBufferCreationInfo, &buffers.frustumCullingDatas),
			   "Failed to create frustum culling datas buffer!");
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffers.frustumShadowMaps),
			   "Failed to create frustum shadow maps buffer!");
}

static inline void CreateActorModelsBuffers()
{
	VulkanTest(lunaCreateBuffer(device, &storageBufferCreationInfo, &buffers.actorModels.instanceData),
			   "Failed to create shaded actor models instance data buffer!");
	VulkanTest(lunaCreateBuffer(device,
								&UNALIGNED_BUFFER_CREATION_INFO,
								&buffers.actorModels.shadedUnculledInstanceIndices),
			   "Failed to create shaded actor models unculled instance indices buffer!");
	VulkanTest(lunaCreateBuffer(device,
								&UNALIGNED_BUFFER_CREATION_INFO,
								&buffers.actorModels.unshadedUnculledInstanceIndices),
			   "Failed to create unshaded actor models unculled instance indices buffer!");
	ListInit(buffers.actorModels.shadedInstanceIndices, LIST_POINTER);
	ListInit(buffers.actorModels.unshadedInstanceIndices, LIST_POINTER);
	ListInit(buffers.actorModels.shadedDrawInfo, LIST_POINTER);
	ListInit(buffers.actorModels.unshadedDrawInfo, LIST_POINTER);
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffers.actorModels.shadedCullingInfo),
			   "Failed to create actor models shaded materials culling info buffer!");
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffers.actorModels.unshadedCullingInfo),
			   "Failed to create actor models unshaded materials culling info buffer!");
}

static inline void CreateActorWallsBuffers()
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
		.size = sizeof(vertices),
		.data = vertices,
		.synchronizationType = LUNA_BUFFER_SYNCHRONIZATION_TYPE_VERTEX_BUFFER,
	};

	const LunaBufferCreationInfo verticesBufferCreationInfo = {
		.size = sizeof(ActorWallVertex) * 12,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
	};
	VulkanTest(lunaCreateBuffer(device, &verticesBufferCreationInfo, &buffers.actorWalls.vertices),
			   "Failed to create actor walls vertex buffer!");
	VulkanTest(lunaWriteDataToBuffer(device, commandBuffer, buffers.actorWalls.vertices, &vertexDataWriteInfo),
			   "Failed to write actor vertex data to buffer!");

	VulkanTest(lunaCreateBuffer(device, &storageBufferCreationInfo, &buffers.actorWalls.instanceData),
			   "Failed to create actor walls instance data buffer!");

	ListInit(buffers.actorWalls.shadedDrawInfo, LIST_POINTER);
	ListInit(buffers.actorWalls.unshadedDrawInfo, LIST_POINTER);
	ListInit(buffers.actorWalls.shadedInstanceIndices, LIST_POINTER);
	ListInit(buffers.actorWalls.unshadedInstanceIndices, LIST_POINTER);

	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffers.actorWalls.shadedCullingInfo),
			   "Failed to create shaded actor walls culling info buffer!");
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffers.actorWalls.unshadedCullingInfo),
			   "Failed to create unshaded actor walls culling info buffer!");
}

static inline void CreateMapModelsBuffers(MapModelsBuffer *buffer)
{
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffer->vertices),
			   "Failed to create shaded map models vertex buffer!");
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffer->indices),
			   "Failed to create shaded map models index buffer!");
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffer->instanceData),
			   "Failed to create shaded map models instance data buffer!");
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffer->unculledShadedDrawInfo),
			   "Failed to create shaded map models unculled draw info buffer!");
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffer->unculledUnshadedDrawInfo),
			   "Failed to create unshaded map models unculled draw info buffer!");
	ListInit(buffer->shadedDrawInfo, LIST_POINTER);
	ListInit(buffer->unshadedDrawInfo, LIST_POINTER);
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffer->shadedCullingInfo),
			   "Failed to create shaded map models materials culling info buffer!");
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffer->unshadedCullingInfo),
			   "Failed to create unshaded map models materials culling info buffer!");
}

static inline void CreateSkyBuffers()
{
	static const size_t SKY_MAX_VERTICES_INIT = 559;
	static const size_t SKY_MAX_INDICES_INIT = 2880;

	const LunaBufferCreationInfo verticesBufferCreationInfo = {
		.size = sizeof(SkyVertex) * SKY_MAX_VERTICES_INIT,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
	};
	VulkanTest(lunaCreateBuffer(device, &verticesBufferCreationInfo, &buffers.sky.vertices),
			   "Failed to create sky vertex buffer!");
	const LunaBufferCreationInfo indicesBufferCreationInfo = {
		.size = sizeof(uint32_t) * SKY_MAX_INDICES_INIT,
		.memoryType = LUNA_MEMORY_TYPE_DEVICE_LOCAL,
	};
	VulkanTest(lunaCreateBuffer(device, &indicesBufferCreationInfo, &buffers.sky.indices),
			   "Failed to create sky index buffer!");
}

static inline void CreateDebugDrawBuffers()
{
#ifdef JPH_DEBUG_RENDERER
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffers.debugDrawLines.buffer),
			   "Failed to create debug draw lines buffer!");
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffers.debugDrawTriangles.buffer),
			   "Failed to create debug draw triangles buffer!");
#endif
}

void CreateBuffers()
{
	storageBufferCreationInfo.alignment = physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;

	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffers.modelVertices),
			   "Failed to create models vertex buffer!");
	VulkanTest(lunaCreateBuffer(device, &UNALIGNED_BUFFER_CREATION_INFO, &buffers.modelIndices),
			   "Failed to create models index buffer!");

	CreateUiBuffers();
	CreateUniformBuffers();
	CreateFrustumsBuffers();
	CreateActorModelsBuffers();
	CreateActorWallsBuffers();
	CreateMapModelsBuffers(&buffers.opaqueMap);
	CreateMapModelsBuffers(&buffers.map);
	CreateModelBuffer(&buffers.viewmodel, "viewmodel");
	CreateSkyBuffers();
	CreateModelBuffer(&buffers.player, "player");
	CreateDebugDrawBuffers();
}

void ResizeDebugDrawBuffers()
{
#ifdef JPH_DEBUG_RENDERER
	VulkanTest(lunaResizeBuffer(device, &buffers.debugDrawLines.buffer, buffers.debugDrawLines.allocatedSize),
			   "Failed to grow debug draw lines buffer!");
	VulkanTest(lunaResizeBuffer(device, &buffers.debugDrawTriangles.buffer, buffers.debugDrawTriangles.allocatedSize),
			   "Failed to grow debug draw triangles buffer!");
#endif
}

static inline uint32_t ImageMetadataBits(const Image *image)
{
	uint32_t bits = 0;
	if (image->filter)
	{
		bits |= (1u << 31u);
	}
	if (image->repeat)
	{
		bits |= (1u << 30u);
	}
	if (image->mipmaps && GetState()->options.mipmaps && GetState()->options.anisotropy)
	{
		bits |= (1u << 29u);
	}
	return bits;
}

void LoadTexture(const Image *image)
{
	const bool useMipmaps = GetState()->options.mipmaps && image->mipmaps;

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
	};
	const size_t index = textures.length;
	LunaImage *lunaImage = ListAdd(textures, LUNA_NULL_HANDLE);
	// TODO: This should only be fatal if the missing texture fails to load
	VulkanTest(lunaCreateImage(device, secondaryCommandBuffer, &imageCreationInfo, lunaImage),
			   "Failed to create texture!");
	imageAssetIdToIndexMap[image->id] = index | ImageMetadataBits(image);

	const LunaDescriptorImageInfo imageInfo = {
		.image = *lunaImage,
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
}
