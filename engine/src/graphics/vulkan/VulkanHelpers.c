//
// Created by Noah on 11/23/2024.
//

#include <assert.h>
#include <cglm/cglm.h>
#include <cglm/clipspace/persp_lh_zo.h>
#include <engine/assets/ShaderLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanInternal.h>
#include <engine/graphics/vulkan/VulkanResources.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Light.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Options.h>
#include <engine/structs/Viewmodel.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <luna/luna.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaImage.h>
#include <luna/lunaTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>

#pragma region variables
bool minimized = false;
LunaDevice device = LUNA_NULL_HANDLE;
VkPhysicalDeviceProperties physicalDeviceProperties = {0};
uint32_t queueFamilyIndex = -1u;
VkQueue queue = VK_NULL_HANDLE;
LunaCommandPool commandPool = LUNA_NULL_HANDLE;
LunaCommandBuffer commandBuffer = LUNA_NULL_HANDLE;
LunaCommandBuffer secondaryCommandBuffer = LUNA_NULL_HANDLE;
LunaSemaphore semaphore = LUNA_NULL_HANDLE;
VkSurfaceKHR surface = VK_NULL_HANDLE;
VkExtent2D swapchainExtent = {0};
VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
LunaRenderPass renderPass = LUNA_NULL_HANDLE;
uint32_t imageAssetIdToIndexMap[MAX_TEXTURES];
TextureSamplers textureSamplers = {
	.linearRepeatAnisotropy = LUNA_NULL_HANDLE,
	.linearNoRepeatAnisotropy = LUNA_NULL_HANDLE,
	.linearRepeatNoAnisotropy = LUNA_NULL_HANDLE,
	.nearestRepeatNoAnisotropy = LUNA_NULL_HANDLE,
	.linearNoRepeatNoAnisotropy = LUNA_NULL_HANDLE,
	.nearestNoRepeatNoAnisotropy = LUNA_NULL_HANDLE,
};
LockingList textures = {0};
LunaDescriptorSetLayout descriptorSetLayout = LUNA_NULL_HANDLE;
LunaDescriptorSet descriptorSet;
Buffers buffers = {0};
Pipelines pipelines = {
	.ui = LUNA_NULL_HANDLE,
#ifdef JPH_DEBUG_RENDERER
	.debugDrawLines = LUNA_NULL_HANDLE,
	.debugDrawTriangles = LUNA_NULL_HANDLE,
#endif
};
uint32_t pendingTasks = 0;
uint32_t skyTextureIndex = 0;
uint32_t lightIndex = 0;
LunaImage shadowMapAtlas = LUNA_NULL_HANDLE;
VkRenderPass shadowMapRenderPass = VK_NULL_HANDLE;
List shadowMapFramebuffers = {0};
List shadowMapImageViews = {0};

static mat4 cameraViewMatrix;
#pragma endregion variables

bool ClearTextureCache()
{
	memset(imageAssetIdToIndexMap, -1, sizeof(*imageAssetIdToIndexMap) * MAX_TEXTURES);
	for (size_t i = 0; i < textures.length; i++)
	{
		lunaDestroyImage(device, (LunaImage)ListGetUint64(textures, i));
	}
	ListFree(textures);
	lunaDestroySampler(device, textureSamplers.linearRepeatAnisotropy);
	lunaDestroySampler(device, textureSamplers.linearNoRepeatAnisotropy);
	lunaDestroySampler(device, textureSamplers.linearRepeatNoAnisotropy);
	lunaDestroySampler(device, textureSamplers.nearestRepeatNoAnisotropy);
	lunaDestroySampler(device, textureSamplers.linearNoRepeatNoAnisotropy);
	lunaDestroySampler(device, textureSamplers.nearestNoRepeatNoAnisotropy);
	return CreateTextureSamplers();
}

VkResult CreateShaderModule(const char *path, const ShaderType shaderType, LunaShaderModule *shaderModule)
{
	Shader *shader = LoadShader(path);
	if (!shader)
	{
		return VK_ERROR_UNKNOWN;
	}
	assert(shader->type == shaderType);
	(void)shaderType;

	const LunaShaderModuleCreationInfo shaderModuleCreationInfo = {
		.creationInfoType = LUNA_SHADER_MODULE_CREATION_INFO_TYPE_SPIRV,
		.creationInfoUnion.spirv.size = sizeof(uint32_t) * shader->spirvLength,
		.creationInfoUnion.spirv.spirv = shader->spirv,
	};
	VulkanTestReturnResult(lunaCreateShaderModule(device, &shaderModuleCreationInfo, shaderModule),
						   "Failed to create shader module!");

	FreeShader(shader);
	return VK_SUCCESS;
}

inline uint32_t TextureIndex(const char *texture)
{
	return ImageIndex(LoadImage(texture));
}

inline uint32_t ImageIndex(const Image *image)
{
	const uint32_t index = imageAssetIdToIndexMap[image->id];
	if (index == -1u)
	{
		if (!LoadTexture(image))
		{
			Error("Failed to load texture into VkImage!");
		}
		return imageAssetIdToIndexMap[image->id];
	}
	return index;
}

inline uint32_t ShadowMapResolution()
{
	switch (GetState()->options.shadowMapQuality)
	{
		case SHADOW_MAP_RESOLUTION_128:
			return 128;
		case SHADOW_MAP_RESOLUTION_256:
			return 256;
		case SHADOW_MAP_RESOLUTION_512:
			return 512;
		case SHADOW_MAP_RESOLUTION_1024:
			return 1024;
		case SHADOW_MAP_RESOLUTION_2048:
			return 2048;
		case SHADOW_MAP_RESOLUTION_4096:
			return 4096;
		default:
			return 0;
	}
}

VkResult CreateShadowMapRenderPass(const Map *map)
{
	const VkDevice vkDevice = lunaGetVkDevice(device);
	if (shadowMapRenderPass != VK_NULL_HANDLE)
	{
		assert(shadowMapFramebuffers.length == shadowMapImageViews.length);
		for (uint32_t i = 0; i < shadowMapFramebuffers.length; i++)
		{
			vkDestroyFramebuffer(vkDevice, ListGetPointer(shadowMapFramebuffers, i), NULL);
			vkDestroyImageView(vkDevice, ListGetPointer(shadowMapImageViews, i), NULL);
		}
		ListFree(shadowMapFramebuffers);
		ListFree(shadowMapImageViews);
		lunaDestroyImage(device, shadowMapAtlas);
		vkDestroyRenderPass(vkDevice, shadowMapRenderPass, NULL);
		shadowMapRenderPass = VK_NULL_HANDLE;
	}
	if (map == NULL)
	{
		return VK_SUCCESS;
	}

	uint32_t shadowMapRegionCount = 0;
	for (uint32_t i = 0; i < map->lightCount; i++)
	{
		Light *light = &map->lights[i];
		switch (light->type)
		{
			case LIGHT_TYPE_SPOT:
				shadowMapRegionCount += 1;
				break;
			case LIGHT_TYPE_POINT:
				shadowMapRegionCount += 6;
				break;
			default:
		}
	}

	if (shadowMapRegionCount == 0)
	{
		return VK_SUCCESS;
	}

	const uint32_t size = 4 * ShadowMapResolution();
	const uint32_t layers = (shadowMapRegionCount - 1) / 16 + 1;

	const VkPipelineStageFlags2 waitStage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
	const LunaCommandBufferSubmitInfo submitInfo = {
		.queue = queue,
		.waitSemaphoreCount = 1,
		.waitSemaphores = &semaphore,
		.waitDstStageMasks = &waitStage,
		.signalSemaphoreCount = 1,
		.signalSemaphores = &semaphore,
	};
	const LunaImageWriteInfo writeInfo = {
		.destinationStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
								VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
		.destinationAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		.submitInfo = &submitInfo,
	};
	const LunaImageCreationInfo shadowMapCreationInfo = {
		.format = VK_FORMAT_D32_SFLOAT,
		.width = size,
		.height = size,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
		.layout = VK_IMAGE_LAYOUT_GENERAL,
		.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
		.writeInfo = writeInfo,
	};
	VulkanTestReturnResult(lunaCreateImageArray(device, commandBuffer, &shadowMapCreationInfo, layers, &shadowMapAtlas),
						   "Failed to create shadow map image!");
	const VkAttachmentDescription attachmentDescription = {
		.format = VK_FORMAT_D32_SFLOAT,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_GENERAL,
		.finalLayout = VK_IMAGE_LAYOUT_GENERAL,
	};
	const VkAttachmentReference attachmentReference = {
		.layout = VK_IMAGE_LAYOUT_GENERAL,
	};
	const VkSubpassDescription subpassDescription = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.pDepthStencilAttachment = &attachmentReference,
	};
	const VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	};
	const VkRenderPassCreateInfo renderPassCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &attachmentDescription,
		.subpassCount = 1,
		.pSubpasses = &subpassDescription,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};
	VulkanTestReturnResult(vkCreateRenderPass(vkDevice, &renderPassCreateInfo, NULL, &shadowMapRenderPass),
						   "Failed to create shadow map render pass!");


	ListInit(shadowMapImageViews, LIST_POINTER);
	ListInit(shadowMapFramebuffers, LIST_POINTER);
	for (uint32_t i = 0; i < layers; i++)
	{
		ListAdd(shadowMapImageViews, VK_NULL_HANDLE);
		const VkImageViewCreateInfo imageViewCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = lunaGetVkImage(shadowMapAtlas),
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT,
			.components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = i,
			.subresourceRange.layerCount = 1,
		};
		VkImageView *imageView = (VkImageView *)&ListGetPointer(shadowMapImageViews, shadowMapImageViews.length - 1);
		VulkanTestReturnResult(vkCreateImageView(vkDevice, &imageViewCreateInfo, NULL, imageView),
							   "Failed to create image view for layer %u of shadow maps!",
							   i);

		ListAdd(shadowMapFramebuffers, VK_NULL_HANDLE);
		const VkFramebufferCreateInfo framebufferCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = shadowMapRenderPass,
			.attachmentCount = 1,
			.pAttachments = imageView,
			.width = size,
			.height = size,
			.layers = 1,
		};
		VulkanTestReturnResult(vkCreateFramebuffer(vkDevice,
												   &framebufferCreateInfo,
												   NULL,
												   (VkFramebuffer *)&ListGetPointer(shadowMapFramebuffers,
																					shadowMapFramebuffers.length - 1)),
							   "Failed to create shadow map frame buffer %u!",
							   i);
	}

	const LunaDescriptorImageInfo shadowMapAtlasImageInfo = {
		.sampler = textureSamplers.nearestNoRepeatNoAnisotropy,
		.image = shadowMapAtlas,
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
	};
	const LunaWriteDescriptorSet shadowMapAtlasDescriptorWrite = {
		.descriptorSet = descriptorSet,
		.bindingName = "Shadow Map Atlas",
		.descriptorCount = 1,
		.imageInfo = &shadowMapAtlasImageInfo,
	};
	lunaWriteDescriptorSets(device, 1, &shadowMapAtlasDescriptorWrite);

	return VK_SUCCESS;
}

// TODO: Make sure this doesn't need changes
VkResult UpdateCameraUniform(const Camera *camera)
{
	mat4 perspectiveMatrix;
	glm_perspective_lh_zo(glm_rad(camera->fov),
						  (float)swapchainExtent.width / (float)swapchainExtent.height,
						  NEAR_Z,
						  FAR_Z,
						  perspectiveMatrix);

	versor rotationQuat;
	QUAT_TO_VERSOR(camera->transform.rotation, rotationQuat);
	versor rotationOffset;
	glm_quatv(rotationOffset, GLM_PIf, GLM_XUP);
	glm_quat_mul(rotationQuat, rotationOffset, rotationQuat);

	vec3 cameraPosition = {camera->transform.position.x, camera->transform.position.y, camera->transform.position.z};
	glm_quat_look(cameraPosition, rotationQuat, cameraViewMatrix);

	CameraUniform uniform;
	glm_mat4_mul(perspectiveMatrix, cameraViewMatrix, uniform.transform);
	uniform.position = camera->transform.position;
	const LunaBufferWriteInfo bufferWriteInfo = {
		.bytes = sizeof(CameraUniform),
		.data = &uniform,
		.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.uniforms.camera, &bufferWriteInfo),
						   "Failed to write camera uniform!");

	return VK_SUCCESS;
}

VkResult UpdateViewModelMatrix(const Viewmodel *viewmodel)
{
	mat4 translationMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_translate(translationMatrix,
				  (vec3){
					  viewmodel->transform.position.x,
					  -viewmodel->transform.position.y,
					  viewmodel->transform.position.z,
				  });

	mat4 rotationMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_rotate(rotationMatrix,
			   JPH_Quat_GetRotationAngle(&viewmodel->transform.rotation, &Vector3_AxisY),
			   (vec3){0.0f, -1.0f, 0.0f});

	glm_mat4_mul(translationMatrix, rotationMatrix, translationMatrix);
	mat4 viewmodelMatrix;
	glm_mat4_inv(cameraViewMatrix, viewmodelMatrix);
	glm_mat4_mul(viewmodelMatrix, translationMatrix, viewmodelMatrix);

	const size_t instanceCount = lunaGetBufferSize(buffers.viewmodel.instanceData) / sizeof(ModelInstanceData);
	for (size_t i = 0; i < instanceCount; i++)
	{
		const LunaBufferWriteInfo writeInfo = {
			.bytes = sizeof(mat4),
			.data = viewmodelMatrix,
			.offset = i * sizeof(ModelInstanceData) + offsetof(ModelInstanceData, transformMatrix),
			.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		};
		VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.viewmodel.instanceData, &writeInfo),
							   "Failed to write viewmodel transform matrix to instance data buffer!");
	}

	return VK_SUCCESS;
}

void EnsureSpaceForUiElements(const size_t quadCount)
{
	if (buffers.ui.freeQuads < quadCount)
	{
		buffers.ui.freeQuads += quadCount + 16;
		buffers.ui.allocatedQuads += quadCount + 16;

		pendingTasks |= PENDING_TASK_UI_BUFFERS_RESIZE_BIT;

		UiVertex *newVertices = realloc(buffers.ui.vertexData, buffers.ui.allocatedQuads * 4 * sizeof(UiVertex));
		CheckAlloc(newVertices);
		buffers.ui.vertexData = newVertices;

		uint32_t *newIndices = realloc(buffers.ui.indexData, buffers.ui.allocatedQuads * 6 * sizeof(uint32_t));
		CheckAlloc(newIndices);
		buffers.ui.indexData = newIndices;
	}
}

void DrawRectInternal(const float ndcStartX,
					  const float ndcStartY,
					  const float ndcEndX,
					  const float ndcEndY,
					  const float startU,
					  const float startV,
					  const float endU,
					  const float endV,
					  const Color *color,
					  const uint32_t textureIndex)
{
	const mat4 vertices = {
		{ndcEndX, ndcStartY, endU, startV},
		{ndcStartX, ndcStartY, startU, startV},
		{ndcStartX, ndcEndY, startU, endV},
		{ndcEndX, ndcEndY, endU, endV},
	};
	DrawQuadInternal(vertices, color, textureIndex);
}

void DrawQuadInternal(const mat4 vertices_posXY_uvZW, const Color *color, const uint32_t textureIndex)
{
	EnsureSpaceForUiElements(1);

	const size_t vertexOffset = (buffers.ui.allocatedQuads - buffers.ui.freeQuads) * 4;
	UiVertex *vertices = buffers.ui.vertexData + vertexOffset;
	uint32_t *indices = buffers.ui.indexData + (buffers.ui.allocatedQuads - buffers.ui.freeQuads) * 6;

	for (uint8_t i = 0; i < 4; i++)
	{
		memcpy(vertices, vertices_posXY_uvZW[i], 16);
		memcpy((char *)(vertices) + 16, &color->r, 16);
		((uint32_t *)(vertices++))[8] = textureIndex;
	}

	indices[0] = vertexOffset;
	indices[1] = vertexOffset + 1;
	indices[2] = vertexOffset + 2;
	indices[3] = vertexOffset;
	indices[4] = vertexOffset + 2;
	indices[5] = vertexOffset + 3;

	buffers.ui.freeQuads--;
}
