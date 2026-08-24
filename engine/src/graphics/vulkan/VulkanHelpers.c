//
// Created by Noah on 11/23/2024.
//

#include <assert.h>
#include <cglm/cglm.h>
#include <cglm/clipspace/ortho_lh_zo.h>
#include <cglm/clipspace/persp_lh_zo.h>
#include <cglm/clipspace/view_lh_zo.h>
#include <engine/assets/ShaderLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/graphics/vulkan/VulkanActors.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanInternal.h>
#include <engine/graphics/vulkan/VulkanResources.h>
#include <engine/helpers/MathEx.h>
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
#include <float.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <luna/luna.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaImage.h>
#include <luna/lunaTypes.h>
#include <math.h>
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
VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
LunaRenderPass renderPass = LUNA_NULL_HANDLE;
uint32_t imageAssetIdToIndexMap[MAX_TEXTURES]; // Gets memset
TextureSamplers textureSamplers = {0};
LockingList textures = {0};
DescriptorSets descriptorSets = {0};
Buffers buffers = {0};
Pipelines pipelines = {0};
uint32_t pendingTasks = 0;
uint32_t skyTextureIndex = 0;
uint32_t shadowMapSlotsAvailable = 0;
ShadowMapPushConstants shadowMapPushConstants = {0};
VkRenderPass shadowMapRenderPass = VK_NULL_HANDLE;
List shadowMaps = {0};
List shadowMapFramebuffers = {0};
List pointLightShadowMapImageViews = {0};
uint32_t frustumIndex = 0;

static CameraUniform uniform;
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

inline uint32_t ShadowMapResolution(const LightType type)
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
		case SHADOW_MAP_RESOLUTION_8192:
			if (type == LIGHT_TYPE_POINT)
			{
				return 4096;
			}
			return 8192;
		case SHADOW_MAP_RESOLUTION_16384:
			if (type == LIGHT_TYPE_POINT)
			{
				return 4096;
			}
			if (type == LIGHT_TYPE_DIRECTIONAL)
			{
				return 8192;
			}
			return 16384;
		default:
			return 0;
	}
}

VkResult CreateShadowMapRenderPass(const Map *map)
{
	const VkDevice vkDevice = lunaGetVkDevice(device);
	if (shadowMapRenderPass != VK_NULL_HANDLE)
	{
		for (uint32_t i = 0; i < shadowMapFramebuffers.length; i++)
		{
			vkDestroyFramebuffer(vkDevice, ListGetPointer(shadowMapFramebuffers, i), NULL);
		}
		ListFree(shadowMapFramebuffers);
		for (uint32_t i = 0; i < shadowMaps.length; i++)
		{
			lunaDestroyImage(device, (LunaImage)ListGetPointer(shadowMaps, i));
		}
		ListFree(shadowMaps);
		for (uint32_t i = 0; i < pointLightShadowMapImageViews.length; i++)
		{
			vkDestroyImageView(vkDevice, ListGetPointer(pointLightShadowMapImageViews, i), NULL);
		}
		ListFree(pointLightShadowMapImageViews);

		vkDestroyRenderPass(vkDevice, shadowMapRenderPass, NULL);
		shadowMapRenderPass = VK_NULL_HANDLE;
	}

	if (map == NULL)
	{
		return VK_SUCCESS;
	}

	const VkAttachmentDescription depthAttachmentDescription = {
		.format = VK_FORMAT_D32_SFLOAT,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_GENERAL,
		.finalLayout = VK_IMAGE_LAYOUT_GENERAL,
	};
	const VkAttachmentReference spotLightDepthAttachmentReference = {
		.layout = VK_IMAGE_LAYOUT_GENERAL,
	};
	const VkSubpassDescription spotLightSubpassDescription = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.pDepthStencilAttachment = &spotLightDepthAttachmentReference,
	};
	const VkSubpassDependency spotLightDependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	};
	const VkRenderPassCreateInfo renderPassCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &depthAttachmentDescription,
		.subpassCount = 1,
		.pSubpasses = &spotLightSubpassDescription,
		.dependencyCount = 1,
		.pDependencies = &spotLightDependency,
	};
	VulkanTestReturnResult(vkCreateRenderPass(vkDevice, &renderPassCreateInfo, NULL, &shadowMapRenderPass),
						   "Failed to create spot light shadow map render pass!");

	ListInit(shadowMaps, LIST_POINTER);
	ListInit(shadowMapFramebuffers, LIST_POINTER);
	ListInit(pointLightShadowMapImageViews, LIST_POINTER);
	uint32_t spotLightCount = 0;
	uint32_t pointLightCount = 0;
	const uint32_t spotLightSize = ShadowMapResolution(LIGHT_TYPE_SPOT);
	const uint32_t pointLightSize = ShadowMapResolution(LIGHT_TYPE_POINT);
	const uint32_t directionalLightSize = ShadowMapResolution(LIGHT_TYPE_DIRECTIONAL);
	const LunaImageWriteInfo depthAttachmentWriteInfo = {
		.destinationStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
								VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
		.destinationAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	};

	const LunaImageCreationInfo spotLightShadowMapCreationInfo = {
		.format = VK_FORMAT_D32_SFLOAT,
		.width = spotLightSize,
		.height = spotLightSize,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
		.layout = VK_IMAGE_LAYOUT_GENERAL,
		.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
		.writeInfo = depthAttachmentWriteInfo,
	};
	const LunaImageCreationInfo pointLightShadowMapCreationInfo = {
		.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		.format = VK_FORMAT_D32_SFLOAT,
		.width = pointLightSize,
		.height = pointLightSize,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
		.layout = VK_IMAGE_LAYOUT_GENERAL,
		.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
		.writeInfo = depthAttachmentWriteInfo,
	};
	const LunaImageCreationInfo directionalLightShadowMapCreationInfo = {
		.format = VK_FORMAT_D32_SFLOAT,
		.width = directionalLightSize,
		.height = directionalLightSize,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
		.layout = VK_IMAGE_LAYOUT_GENERAL,
		.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
		.writeInfo = depthAttachmentWriteInfo,
	};
	for (uint32_t i = 0; i < map->lightCount; i++)
	{
		Light *light = &map->lights[i];

		ListAdd(shadowMaps, LUNA_NULL_HANDLE);
		LunaImage *image = (LunaImage *)&ListGetPointer(shadowMaps, shadowMaps.length - 1);
		LunaDescriptorImageInfo shadowMapImageInfos[4] = {
			{
				.sampler = textureSamplers.shadowMaps,
				.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
			},
			{
				.sampler = textureSamplers.shadowMaps,
				.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
			},
			{
				.sampler = textureSamplers.shadowMaps,
				.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
			},
			{
				.sampler = textureSamplers.shadowMaps,
				.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
			},
		};
		LunaWriteDescriptorSet shadowMapDescriptorWrite = {
			.bindingName = "Shadow Maps",
			.imageInfos = shadowMapImageInfos,
		};
		if (light->type == LIGHT_TYPE_SPOT)
		{
			VulkanTestReturnResult(lunaCreateImage(device, commandBuffer, &spotLightShadowMapCreationInfo, image),
								   "Failed to create spot light shadow map image!");

			ListAdd(shadowMapFramebuffers, VK_NULL_HANDLE);
			const VkImageView imageView = lunaGetVkImageView(*image);
			const VkFramebufferCreateInfo framebufferCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = shadowMapRenderPass,
				.attachmentCount = 1,
				.pAttachments = &imageView,
				.width = spotLightSize,
				.height = spotLightSize,
				.layers = 1,
			};
			VulkanTestReturnResult(vkCreateFramebuffer(vkDevice,
													   &framebufferCreateInfo,
													   NULL,
													   (VkFramebuffer *)&ListGetPointer(shadowMapFramebuffers,
																						shadowMapFramebuffers.length -
																								1)),
								   "Failed to create spot light shadow map framebuffer!");

			shadowMapDescriptorWrite.descriptorSet = descriptorSets.spotLightShadowMaps.set;
			shadowMapDescriptorWrite.descriptorArrayElement = spotLightCount++;
			shadowMapDescriptorWrite.descriptorCount = 1;
			shadowMapImageInfos->image = *image;
		} else if (light->type == LIGHT_TYPE_DIRECTIONAL)
		{
			for (uint32_t cascade = 0; cascade < 4; cascade++)
			{
				ListAdd(shadowMaps, LUNA_NULL_HANDLE);
				image = (LunaImage *)&ListGetPointer(shadowMaps, shadowMaps.length - 1);
				VulkanTestReturnResult(lunaCreateImage(device,
													   commandBuffer,
													   &directionalLightShadowMapCreationInfo,
													   image),
									   "Failed to create directional light shadow map image!");

				ListAdd(shadowMapFramebuffers, VK_NULL_HANDLE);
				const VkImageView imageView = lunaGetVkImageView(*image);
				const VkFramebufferCreateInfo framebufferCreateInfo = {
					.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.renderPass = shadowMapRenderPass,
					.attachmentCount = 1,
					.pAttachments = &imageView,
					.width = directionalLightSize,
					.height = directionalLightSize,
					.layers = 1,
				};
				VulkanTestReturnResult(vkCreateFramebuffer(vkDevice,
														   &framebufferCreateInfo,
														   NULL,
														   (VkFramebuffer *)&ListGetPointer(shadowMapFramebuffers,
																							shadowMapFramebuffers
																											.length -
																									1)),
									   "Failed to create directional light shadow map framebuffer!");

				shadowMapImageInfos[cascade].image = *image;
			}
			shadowMapDescriptorWrite.descriptorSet = descriptorSets.common.set;
			shadowMapDescriptorWrite.descriptorArrayElement = 0;
			shadowMapDescriptorWrite.descriptorCount = 4;
		} else if (light->type == LIGHT_TYPE_POINT)
		{
			VulkanTestReturnResult(lunaCreateImageCube(device, commandBuffer, &pointLightShadowMapCreationInfo, image),
								   "Failed to create point light shadow map image!");
			for (uint32_t layer = 0; layer < 6; layer++)
			{
				ListAdd(shadowMapFramebuffers, VK_NULL_HANDLE);
				ListAdd(pointLightShadowMapImageViews, VK_NULL_HANDLE);
				VkImageView *imageView = (VkImageView *)&ListGetPointer(pointLightShadowMapImageViews,
																		pointLightShadowMapImageViews.length - 1);
				const VkImageViewCreateInfo imageViewCreateInfo = {
					.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					.image = lunaGetVkImage(*image),
					.viewType = VK_IMAGE_VIEW_TYPE_2D,
					.format = VK_FORMAT_D32_SFLOAT,
					.components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
					.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
					.subresourceRange.levelCount = 1,
					.subresourceRange.baseArrayLayer = layer,
					.subresourceRange.layerCount = 1,
				};
				VulkanTestReturnResult(vkCreateImageView(vkDevice, &imageViewCreateInfo, NULL, imageView),
									   "Failed to create image view for point light shadow map!");
				const VkFramebufferCreateInfo framebufferCreateInfo = {
					.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.renderPass = shadowMapRenderPass,
					.attachmentCount = 1,
					.pAttachments = imageView,
					.width = pointLightSize,
					.height = pointLightSize,
					.layers = 1,
				};
				VulkanTestReturnResult(vkCreateFramebuffer(vkDevice,
														   &framebufferCreateInfo,
														   NULL,
														   (VkFramebuffer *)&ListGetPointer(shadowMapFramebuffers,
																							shadowMapFramebuffers
																											.length -
																									1)),
									   "Failed to create point light shadow map framebuffer!");
			}
			shadowMapDescriptorWrite.descriptorSet = descriptorSets.pointLightShadowMaps.set;
			shadowMapDescriptorWrite.descriptorArrayElement = pointLightCount++;
			shadowMapDescriptorWrite.descriptorCount = 1;
			shadowMapImageInfos->image = *image;
		}
		lunaWriteDescriptorSets(device, 1, &shadowMapDescriptorWrite);
	}

	return VK_SUCCESS;
}

VkResult UpdateCameraUniform(Camera *camera)
{
	if (camera->recomputeCachedData)
	{
		const Vector2 windowSize = ActualWindowSizeIgnoreDPI();
		glm_perspective_lh_zo(glm_rad(camera->fov),
							  windowSize.x / windowSize.y,
							  camera->farPlane,
							  camera->nearPlane,
							  camera->projectionMatrix);

		mat4 transposed;
		glm_mat4_transpose_to(camera->projectionMatrix, transposed);
		vec4 frustumX;
		vec4 frustumY;
		glm_vec4_add(transposed[3], transposed[0], frustumX);
		glm_vec4_add(transposed[3], transposed[1], frustumY);
		glm_plane_normalize(frustumX);
		glm_plane_normalize(frustumY);
		camera->frustumPlanes[0] = frustumX[0];
		camera->frustumPlanes[1] = frustumX[2];
		camera->frustumPlanes[2] = frustumY[1];
		camera->frustumPlanes[3] = frustumY[2];

		uniform.nearPlane = camera->nearPlane;
		uniform.farPlane = camera->farPlane;
		uniform.frustumPlanes[0] = camera->frustumPlanes[0];
		uniform.frustumPlanes[1] = camera->frustumPlanes[1];
		uniform.frustumPlanes[2] = camera->frustumPlanes[2];
		uniform.frustumPlanes[3] = camera->frustumPlanes[3];
	}

	versor rotationQuat;
	QUAT_TO_VERSOR(camera->transform.rotation, rotationQuat);
	versor rotationOffset;
	glm_quatv(rotationOffset, GLM_PIf, GLM_XUP);
	glm_quat_mul(rotationQuat, rotationOffset, rotationQuat);

	vec3 cameraPosition = {camera->transform.position.x, camera->transform.position.y, camera->transform.position.z};
	glm_quat_look(cameraPosition, rotationQuat, uniform.view);

	glm_mat4_mul(camera->projectionMatrix, uniform.view, uniform.transform);
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
	glm_mat4_inv(uniform.view, viewmodelMatrix);
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

// TODO: Optimize this function
VkResult UpdateDirectionalLightCascades(const Camera *camera, const Light *light)
{
	static const float LAMBDA = 0.95f; // Adjusts the range of each split. Tweak to find optimal values

	const float nearPlane = camera->nearPlane;
	const float farPlane = camera->farPlane;

	const float range = farPlane - nearPlane;
	const float ratio = farPlane / nearPlane;

	float depths[4];
	mat4 matrices[4];
	Frustum frustums[4];
	float previousDistance = 0.0f;
	for (uint32_t i = 0; i < 4; i++)
	{
		const float p = (float)(i + 1) / 4.0f;
		const float v = nearPlane + range * p;
		const float d = LAMBDA * (nearPlane * powf(ratio, p) - v) + v;
		const float distance = (d - nearPlane) / range;

		vec4 frustumCorners[8] = {
			{-1.0f, 1.0f, 1.0f, 1.0f},
			{1.0f, 1.0f, 1.0f, 1.0f},
			{1.0f, -1.0f, 1.0f, 1.0f},
			{-1.0f, -1.0f, 1.0f, 1.0f},
			{-1.0f, 1.0f, 0.0f, 1.0f},
			{1.0f, 1.0f, 0.0f, 1.0f},
			{1.0f, -1.0f, 0.0f, 1.0f},
			{-1.0f, -1.0f, 0.0f, 1.0f},
		};

		// Project frustum corners into world space
		mat4 cameraInverseTransform;
		glm_mat4_inv(uniform.transform, cameraInverseTransform);
		for (uint32_t j = 0; j < 8; j++)
		{
			vec4 inverseCorner;
			glm_mat4_mulv(cameraInverseTransform, frustumCorners[j], inverseCorner);
			glm_vec4_divs(inverseCorner, inverseCorner[3], frustumCorners[j]);
		}

		for (uint32_t j = 0; j < 4; j++)
		{
			vec3 frustumDistance;
			glm_vec3_sub(frustumCorners[j + 4], frustumCorners[j], frustumDistance);
			vec4 scaledFrustumDistance;
			scaledFrustumDistance[3] = 0;
			glm_vec3_scale(frustumDistance, distance, scaledFrustumDistance);
			glm_vec4_add(frustumCorners[j], scaledFrustumDistance, frustumCorners[j + 4]);
			glm_vec3_scale(frustumDistance, previousDistance, scaledFrustumDistance);
			glm_vec4_add(frustumCorners[j], scaledFrustumDistance, frustumCorners[j]);
		}

		vec3 frustumCenter = GLM_VEC3_ZERO_INIT;
		for (uint32_t j = 0; j < 8; j++)
		{
			glm_vec3_add(frustumCenter, frustumCorners[j], frustumCenter);
		}
		glm_vec3_divs(frustumCenter, 8.0f, frustumCenter);

		float radius = 0.0f;
		for (uint32_t j = 0; j < 8; j++)
		{
			radius = max(radius, glm_vec3_distance(frustumCorners[j], frustumCenter));
		}
		radius = ceilf(radius * 16.0f) / 16.0f;

		vec3 eye;
		glm_vec3_scale(VECTOR3_TO_VEC3(light->negativeForwardDirection), radius, eye);
		glm_vec3_add(frustumCenter, eye, eye);
		mat4 viewMatrix;
		const bool yAligned = fabsf(light->negativeForwardDirection.x) < FLT_EPSILON &&
							  fabsf(light->negativeForwardDirection.z) < FLT_EPSILON;
		glm_lookat_lh_zo(eye, frustumCenter, yAligned ? GLM_XUP : GLM_YUP, viewMatrix);
		mat4 projectionMatrix;
		glm_ortho_lh_zo(radius, -radius, radius, -radius, radius * 2, 0, projectionMatrix);

		depths[i] = nearPlane + distance * range;
		glm_mat4_mul(projectionMatrix, viewMatrix, matrices[i]);

		mat4 transposed;
		glm_mat4_transpose_to(projectionMatrix, transposed);
		vec4 frustumX;
		vec4 frustumY;
		glm_vec4_add(transposed[3], transposed[0], frustumX);
		glm_vec4_add(transposed[3], transposed[1], frustumY);
		glm_plane_normalize(frustumX);
		glm_plane_normalize(frustumY);

		glm_mat4_copy(viewMatrix, frustums[i].viewMatrix);
		frustums[i].nearPlane = 0;
		frustums[i].farPlane = radius * 2;
		frustums[i].frustumPlanes[0] = frustumX[0];
		frustums[i].frustumPlanes[1] = frustumX[2];
		frustums[i].frustumPlanes[2] = frustumY[1];
		frustums[i].frustumPlanes[3] = frustumY[2];

		previousDistance = distance;
	}

	const LunaBufferWriteInfo depthsWriteInfo = {
		.bytes = sizeof(float) * 4,
		.data = depths,
		.offset = sizeof(uint32_t),
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.uniforms.lights, &depthsWriteInfo),
						   "Failed to write directional light cascade depths to buffer!");
	const LunaBufferWriteInfo matricesWriteInfo = {
		.bytes = sizeof(mat4) * 4,
		.data = matrices,
		.offset = sizeof(uint32_t) + sizeof(float) * 4,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.uniforms.lights, &matricesWriteInfo),
						   "Failed to write directional light cascade transform matrices to buffer!");
	const LunaBufferWriteInfo frustumsWriteInfo = {
		.bytes = sizeof(frustums),
		.data = frustums,
		.offset = sizeof(Frustum) * frustumIndex,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.frustums, &frustumsWriteInfo),
						   "Failed to write directional light frustums to buffer!");

	return VK_SUCCESS;
}

VkResult CullModels()
{
	SetActorInstanceIndices();

	const LunaBuffer bufferHandles[] = {
		buffers.opaqueMap.shadedDrawInfo,
		buffers.opaqueMap.unshadedDrawInfo,
		buffers.map.shadedDrawInfo,
		buffers.map.unshadedDrawInfo,
		buffers.actorWalls.shadedDrawInfo,
		buffers.actorWalls.unshadedDrawInfo,
		buffers.actorWalls.shadedInstanceIndices,
		buffers.actorWalls.unshadedInstanceIndices,
		buffers.actorModels.shadedDrawInfo,
		buffers.actorModels.unshadedDrawInfo,
		buffers.actorModels.shadedInstanceIndices,
		buffers.actorModels.unshadedInstanceIndices,
	};
	const LunaMultiBufferMemoryBarrier preClearMemoryBarrier = {
		.sourceStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
		.sourceAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		.destinationAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
		.bufferCount = sizeof(bufferHandles) / sizeof(*bufferHandles),
		.buffers = bufferHandles,
		.size = sizeof(uint32_t),
	};
	const LunaDependencyInfo preClearDependencyInfo = {
		.multiBufferMemoryBarrierCount = 1,
		.multiBufferMemoryBarriers = &preClearMemoryBarrier,
	};
	VulkanTestReturnResult(lunaPipelineBarrier(device, commandBuffer, &preClearDependencyInfo),
						   "Failed to insert pipeline barrier before clearing culling data!");

	const LunaDescriptorSetBindInfo clearDescriptorSetBindInfo = {
		.descriptorSetCount = 1,
		.descriptorSets = &descriptorSets.culling.set,
	};
	const LunaDispatchInfo clearDispatchInfo = {
		.pipeline = pipelines.clearCullingData,
		.descriptorSetBindInfo = &clearDescriptorSetBindInfo,
	};
	VulkanTestReturnResult(lunaDispatch(device, commandBuffer, &clearDispatchInfo),
						   "Failed to dispatch culling data clear shader!");

	const LunaMultiBufferMemoryBarrier preDispatchMemoryBarrier = {
		.sourceStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		.sourceAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		.destinationAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
		.bufferCount = sizeof(bufferHandles) / sizeof(*bufferHandles),
		.buffers = bufferHandles,
		.size = sizeof(uint32_t),
	};
	const LunaDependencyInfo preDispatchDependencyInfo = {
		.multiBufferMemoryBarrierCount = 1,
		.multiBufferMemoryBarriers = &preDispatchMemoryBarrier,
	};
	VulkanTestReturnResult(lunaPipelineBarrier(device, commandBuffer, &preDispatchDependencyInfo),
						   "Failed to insert pipeline barrier before culling shader!");

	VulkanTestReturnResult(lunaPushConstantsCompute(device, commandBuffer, pipelines.culling),
						   "Failed to push constants for culling pipeline!");
	const LunaDescriptorSet descriptorSetHandles[] = {descriptorSets.common.set, descriptorSets.culling.set};
	const LunaDescriptorSetBindInfo descriptorSetBindInfo = {
		.descriptorSetCount = 2,
		.descriptorSets = descriptorSetHandles,
	};
	const LunaDispatchInfo dispatchInfo = {
		.pipeline = pipelines.culling,
		.descriptorSetBindInfo = &descriptorSetBindInfo,
		.groupCountX = 16,
		.groupCountY = 8,
	};
	VulkanTestReturnResult(lunaDispatch(device, commandBuffer, &dispatchInfo), "Failed to dispatch culling shader!");

	const LunaMultiBufferMemoryBarrier postDispatchMemoryBarrier = {
		.sourceStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		.sourceAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT |
								VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT |
								VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
		.destinationAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT |
								 VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT |
								 VK_ACCESS_2_SHADER_READ_BIT,
		.bufferCount = sizeof(bufferHandles) / sizeof(*bufferHandles),
		.buffers = bufferHandles,
	};
	const LunaDependencyInfo postDispatchDependencyInfo = {
		.multiBufferMemoryBarrierCount = 1,
		.multiBufferMemoryBarriers = &postDispatchMemoryBarrier,
	};
	VulkanTestReturnResult(lunaPipelineBarrier(device, commandBuffer, &postDispatchDependencyInfo),
						   "Failed to insert pipeline barrier after culling shader!");

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
