//
// Created by Noah on 12/18/2024.
//

#include <cglm/cglm.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanResources.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/threads/LodThread.h>
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

VkResult CreateUiBuffers()
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

VkResult CreateUniformBuffers()
{
	const LunaBufferCreationInfo cameraUniformBufferCreationInfo = {
		.size = sizeof(CameraUniform),
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &cameraUniformBufferCreationInfo, &buffers.uniforms.camera),
						   "Failed to create camera uniform buffer!");
	const LunaBufferCreationInfo lightingBufferCreationInfo = {
		.size = sizeof(float) * 4, // r, g, b, a
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &lightingBufferCreationInfo, &buffers.uniforms.lighting),
						   "Failed to create lighting uniform buffer!");
	const LunaBufferCreationInfo fogBufferCreationInfo = {
		.size = sizeof(Color) + sizeof(float) * 2, // fogColor, fogStart, fogEnd
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &fogBufferCreationInfo, &buffers.uniforms.fog),
						   "Failed to create fog uniform buffer!");

	return VK_SUCCESS;
}

VkResult CreateMapBuffers()
{
	const LunaBufferCreationInfo verticesBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &verticesBufferCreationInfo, &buffers.map.vertices),
						   "Failed to create shaded map vertex buffer!");
	const LunaBufferCreationInfo indicesBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &indicesBufferCreationInfo, &buffers.map.indices),
						   "Failed to create shaded map index buffer!");
	const LunaBufferCreationInfo instanceDataBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &instanceDataBufferCreationInfo, &buffers.map.instanceData),
						   "Failed to create shaded map instance data buffer!");
	const LunaBufferCreationInfo shadedDrawInfoBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &shadedDrawInfoBufferCreationInfo, &buffers.map.shadedDrawInfo),
						   "Failed to create shaded map draw info buffer!");
	const LunaBufferCreationInfo unshadedDrawInfoBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &unshadedDrawInfoBufferCreationInfo, &buffers.map.unshadedDrawInfo),
						   "Failed to create unshaded map draw info buffer!");

	return VK_SUCCESS;
}

VkResult CreateSkyBuffers()
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

VkResult CreateViewmodelBuffers()
{
	const LunaBufferCreationInfo verticesBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &verticesBufferCreationInfo, &buffers.viewmodel.vertices),
						   "Failed to create shaded viewmodel vertex buffer!");
	const LunaBufferCreationInfo indicesBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &indicesBufferCreationInfo, &buffers.viewmodel.indices),
						   "Failed to create shaded viewmodel index buffer!");
	const LunaBufferCreationInfo instanceDataBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &instanceDataBufferCreationInfo, &buffers.viewmodel.instanceData),
						   "Failed to create shaded viewmodel instance data buffer!");
	const LunaBufferCreationInfo shadedDrawInfoBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device,
											&shadedDrawInfoBufferCreationInfo,
											&buffers.viewmodel.shadedDrawInfo),
						   "Failed to create shaded viewmodel draw info buffer!");
	const LunaBufferCreationInfo unshadedDrawInfoBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device,
											&unshadedDrawInfoBufferCreationInfo,
											&buffers.viewmodel.unshadedDrawInfo),
						   "Failed to create unshaded viewmodel draw info buffer!");

	return VK_SUCCESS;
}

VkResult CreateActorModelBuffers()
{
	const LunaBufferCreationInfo verticesBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &verticesBufferCreationInfo, &buffers.actorModels.vertices),
						   "Failed to create shaded actor models vertex buffer!");
	const LunaBufferCreationInfo indicesBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &indicesBufferCreationInfo, &buffers.actorModels.indices),
						   "Failed to create shaded actor models index buffer!");
	const LunaBufferCreationInfo instanceDataBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device, &instanceDataBufferCreationInfo, &buffers.actorModels.instanceData),
						   "Failed to create shaded actor models instance data buffer!");
	const LunaBufferCreationInfo shadedDrawInfoBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device,
											&shadedDrawInfoBufferCreationInfo,
											&buffers.actorModels.shadedDrawInfo),
						   "Failed to create shaded actor models draw info buffer!");
	const LunaBufferCreationInfo unshadedDrawInfoBufferCreationInfo = {
		.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(device,
											&unshadedDrawInfoBufferCreationInfo,
											&buffers.actorModels.unshadedDrawInfo),
						   "Failed to create unshaded actor models draw info buffer!");

	return VK_SUCCESS;
}

VkResult CreateDebugDrawBuffers()
{
#ifdef JPH_DEBUG_RENDERER
	const LunaBufferCreationInfo linesVertexBuffer = {
		.size = buffers.debugDrawLines.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&linesVertexBuffer, &buffers.debugDrawLines.vertices.buffer),
						   "Failed to create debug draw lines buffer!");
	buffers.debugDrawLines.vertices.data = malloc(buffers.debugDrawLines.vertices.allocatedSize);
	CheckAlloc(buffers.debugDrawLines.vertices.data);

	const LunaBufferCreationInfo vertexVertexBuffer = {
		.size = buffers.debugDrawTriangles.vertices.allocatedSize,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTestReturnResult(lunaCreateBuffer(&vertexVertexBuffer, &buffers.debugDrawTriangles.vertices.buffer),
						   "Failed to create debug draw triangles buffer!");
	buffers.debugDrawTriangles.vertices.data = malloc(buffers.debugDrawTriangles.vertices.allocatedSize);
	CheckAlloc(buffers.debugDrawTriangles.vertices.data);
#endif

	return VK_SUCCESS;
}

VkResult ResizeDebugDrawBuffers()
{
#ifdef JPH_DEBUG_RENDERER
	if (buffers.debugDrawLines.vertices.allocatedSize < buffers.debugDrawLines.vertices.bytesUsed)
	{
		VulkanTestReturnResult(lunaResizeBuffer(&buffers.debugDrawLines.vertices.buffer,
												buffers.debugDrawLines.vertices.bytesUsed),
							   "Failed to resize debug draw lines buffer!");
		buffers.debugDrawLines.vertices.allocatedSize = buffers.debugDrawLines.vertices.bytesUsed;
	}
	void *newData = realloc(buffers.debugDrawLines.vertices.data, buffers.debugDrawLines.vertices.allocatedSize);
	CheckAlloc(newData);
	buffers.debugDrawLines.vertices.data = newData;
	buffers.debugDrawLines.shouldResize = false;

	if (buffers.debugDrawTriangles.vertices.allocatedSize < buffers.debugDrawTriangles.vertices.bytesUsed)
	{
		VulkanTestReturnResult(lunaResizeBuffer(&buffers.debugDrawTriangles.vertices.buffer,
												buffers.debugDrawTriangles.vertices.bytesUsed),
							   "Failed to resize debug draw triangles buffer!");
		buffers.debugDrawTriangles.vertices.allocatedSize = buffers.debugDrawTriangles.vertices.bytesUsed;
	}

	newData = realloc(buffers.debugDrawTriangles.vertices.data, buffers.debugDrawTriangles.vertices.allocatedSize);
	CheckAlloc(newData);
	buffers.debugDrawTriangles.vertices.data = newData;
	buffers.debugDrawTriangles.shouldResize = false;
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
	const LunaImageCreationInfo imageCreationInfo = {
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.width = image->width,
		.height = image->height,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.writeInfo.bytes = image->width * image->height * sizeof(uint32_t),
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
	VulkanTest(lunaCreateImage(device, secondaryCommandBuffer, &imageCreationInfo, &lunaImage), "Failed to create texture!");
	imageAssetIdToIndexMap[image->id] = index;
	ListAdd(textures, lunaImage);

	const LunaDescriptorImageInfo imageInfo = {
		.image = lunaImage,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};
	const LunaWriteDescriptorSet writeDescriptor = {
		.descriptorSet = descriptorSet,
		.bindingName = "Textures",
		.descriptorArrayElement = index,
		.descriptorCount = 1,
		.imageInfo = &imageInfo,
	};
	lunaWriteDescriptorSets(device, 1, &writeDescriptor);

	return true;
}
