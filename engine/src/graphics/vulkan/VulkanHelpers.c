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
List perFrustumBuffersHandles = {0};
uint32_t frustumCount = 0;
FrustumCullingData *frustums = NULL;
uint32_t actorModelsDrawInfoCount = 0;
uint32_t maximumCulledInstanceCount = 0;
uint32_t lightCount = 0;
uint32_t lightmapTextureSize = 0;

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

inline uint32_t ShadowMapResolution(void)
{
	switch (GetState()->options.shadowMapQuality)
	{
		case SHADOW_MAP_RESOLUTION_128:
			lightmapTextureSize = 128;
			break;
		case SHADOW_MAP_RESOLUTION_256:
			lightmapTextureSize = 256;
			break;
		case SHADOW_MAP_RESOLUTION_512:
			lightmapTextureSize = 512;
			break;
		case SHADOW_MAP_RESOLUTION_1024:
			lightmapTextureSize = 1024;
			break;
		case SHADOW_MAP_RESOLUTION_2048:
			lightmapTextureSize = 2048;
			break;
		case SHADOW_MAP_RESOLUTION_4096:
			lightmapTextureSize = 4096;
			break;
		default:
			lightmapTextureSize = 0;
			break;
	}
	return lightmapTextureSize;
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

		vkDestroyRenderPass(vkDevice, shadowMapRenderPass, NULL);
		shadowMapRenderPass = VK_NULL_HANDLE;
	}

	if (map == NULL || GetState()->options.shadowMapQuality == SHADOW_MAP_RESOLUTION_DISABLED)
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
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
	};
	const VkAttachmentReference depthAttachmentReference = {
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	const VkSubpassDescription subpassDescription = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.pDepthStencilAttachment = &depthAttachmentReference,
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
		.pAttachments = &depthAttachmentDescription,
		.subpassCount = 1,
		.pSubpasses = &subpassDescription,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};
	VulkanTestReturnResult(vkCreateRenderPass(vkDevice, &renderPassCreateInfo, NULL, &shadowMapRenderPass),
						   "Failed to create shadow map render pass!");

	ListInit(shadowMaps, LIST_POINTER);
	ListInit(shadowMapFramebuffers, LIST_POINTER);
	uint32_t shadowMapCount = 0;
	const uint32_t lightSize = ShadowMapResolution();
	const LunaImageWriteInfo depthAttachmentWriteInfo = {
		.destinationStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
								VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
		.destinationAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	};

	const LunaImageCreationInfo shadowMapCreationInfo = {
		.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		.format = VK_FORMAT_D32_SFLOAT,
		.width = lightSize,
		.height = lightSize,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
		.writeInfo = depthAttachmentWriteInfo,
	};
	for (uint32_t i = 0; i < map->lightCount; i++)
	{
		Light *light = &map->lights[i];

		LunaImage *image = LUNA_NULL_HANDLE;
		LunaDescriptorImageInfo shadowMapImageInfos[6] = {
			{
				.sampler = textureSamplers.shadowMaps,
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
			},
			{
				.sampler = textureSamplers.shadowMaps,
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
			},
			{
				.sampler = textureSamplers.shadowMaps,
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
			},
			{
				.sampler = textureSamplers.shadowMaps,
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
			},
			{
				.sampler = textureSamplers.shadowMaps,
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
			},
			{
				.sampler = textureSamplers.shadowMaps,
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
			},
		};
		LunaWriteDescriptorSet shadowMapDescriptorWrite = {
			.bindingName = "Shadow Maps",
			.imageInfos = shadowMapImageInfos,
		};
		if (light->type == LIGHT_TYPE_SPOT)
		{
			image = ListAdd(shadowMaps, LUNA_NULL_HANDLE);
			VulkanTestReturnResult(lunaCreateImage(device, commandBuffer, &shadowMapCreationInfo, image),
								   "Failed to create spot light shadow map image!");

			VkFramebuffer *framebuffer = ListAdd(shadowMapFramebuffers, VK_NULL_HANDLE);
			const VkImageView imageView = lunaGetVkImageView(*image);
			const VkFramebufferCreateInfo framebufferCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = shadowMapRenderPass,
				.attachmentCount = 1,
				.pAttachments = &imageView,
				.width = lightSize,
				.height = lightSize,
				.layers = 1,
			};
			VulkanTestReturnResult(vkCreateFramebuffer(vkDevice, &framebufferCreateInfo, NULL, framebuffer),
								   "Failed to create spot light shadow map framebuffer!");

			shadowMapDescriptorWrite.descriptorSet = descriptorSets.shadowMaps.set;
			shadowMapDescriptorWrite.descriptorArrayElement = shadowMapCount++;
			shadowMapDescriptorWrite.descriptorCount = 1;
			shadowMapImageInfos->image = *image;
		} else if (light->type == LIGHT_TYPE_DIRECTIONAL)
		{
			for (uint32_t cascade = 0; cascade < 4; cascade++)
			{
				image = ListAdd(shadowMaps, LUNA_NULL_HANDLE);
				VulkanTestReturnResult(lunaCreateImage(device, commandBuffer, &shadowMapCreationInfo, image),
									   "Failed to create directional light shadow map image!");

				VkFramebuffer *framebuffer = ListAdd(shadowMapFramebuffers, VK_NULL_HANDLE);
				const VkImageView imageView = lunaGetVkImageView(*image);
				const VkFramebufferCreateInfo framebufferCreateInfo = {
					.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.renderPass = shadowMapRenderPass,
					.attachmentCount = 1,
					.pAttachments = &imageView,
					.width = lightSize,
					.height = lightSize,
					.layers = 1,
				};
				VulkanTestReturnResult(vkCreateFramebuffer(vkDevice, &framebufferCreateInfo, NULL, framebuffer),
									   "Failed to create directional light shadow map framebuffer!");

				shadowMapImageInfos[cascade].image = *image;
			}
			shadowMapDescriptorWrite.descriptorSet = descriptorSets.common.set;
			shadowMapDescriptorWrite.descriptorArrayElement = 0;
			shadowMapDescriptorWrite.descriptorCount = 4;
		} else if (light->type == LIGHT_TYPE_POINT)
		{
			for (uint32_t layer = 0; layer < 6; layer++)
			{
				image = ListAdd(shadowMaps, LUNA_NULL_HANDLE);
				VulkanTestReturnResult(lunaCreateImage(device, commandBuffer, &shadowMapCreationInfo, image),
									   "Failed to create point light shadow map image!");

				VkFramebuffer *framebuffer = ListAdd(shadowMapFramebuffers, VK_NULL_HANDLE);
				const VkImageView imageView = lunaGetVkImageView(*image);
				const VkFramebufferCreateInfo framebufferCreateInfo = {
					.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.renderPass = shadowMapRenderPass,
					.attachmentCount = 1,
					.pAttachments = &imageView,
					.width = lightSize,
					.height = lightSize,
					.layers = 1,
				};
				VulkanTestReturnResult(vkCreateFramebuffer(vkDevice, &framebufferCreateInfo, NULL, framebuffer),
									   "Failed to create point light shadow map framebuffer!");
				shadowMapImageInfos[layer].image = *image;
			}
			shadowMapDescriptorWrite.descriptorSet = descriptorSets.shadowMaps.set;
			shadowMapDescriptorWrite.descriptorArrayElement = shadowMapCount;
			shadowMapDescriptorWrite.descriptorCount = 6;
			shadowMapCount += 6;
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

		camera->frustumPlanes[1] = 1 / Vector2Length(v2(camera->projectionMatrix[0][0], 1));
		camera->frustumPlanes[3] = 1 / Vector2Length(v2(camera->projectionMatrix[1][1], 1));
		camera->frustumPlanes[0] = camera->projectionMatrix[0][0] * camera->frustumPlanes[1];
		camera->frustumPlanes[2] = camera->projectionMatrix[1][1] * camera->frustumPlanes[3];

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
	if (light == NULL || GetState()->options.shadowMapQuality == SHADOW_MAP_RESOLUTION_DISABLED)
	{
		return VK_SUCCESS;
	}

	static const float LAMBDA = 0.95f; // Adjusts the range of each split. Tweak to find optimal values

	const float nearPlane = camera->nearPlane;
	const float farPlane = camera->farPlane;

	const float range = farPlane - nearPlane;
	const float ratio = farPlane / nearPlane;

	const Vector2 windowSize = ActualWindowSizeIgnoreDPI();
	mat4 transformMatrix;
	glm_perspective_lh_zo(glm_rad(camera->fov), windowSize.x / windowSize.y, farPlane, nearPlane, transformMatrix);
	glm_mat4_mul(transformMatrix, uniform.view, transformMatrix);
	glm_mat4_inv(transformMatrix, transformMatrix);
	vec4 projectedCorners[8] = {
		{-1.0f, 1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{1.0f, -1.0f, 1.0f, 1.0f},
		{-1.0f, -1.0f, 1.0f, 1.0f},
		{-1.0f, 1.0f, 0.0f, 1.0f},
		{1.0f, 1.0f, 0.0f, 1.0f},
		{1.0f, -1.0f, 0.0f, 1.0f},
		{-1.0f, -1.0f, 0.0f, 1.0f},
	};
	for (uint32_t j = 0; j < 8; j++)
	{
		vec4 inverseCorner;
		glm_mat4_mulv(transformMatrix, projectedCorners[j], inverseCorner);
		glm_vec4_divs(inverseCorner, inverseCorner[3], projectedCorners[j]);
	}

	float depths[4];
	mat4 matrices[4];
	float previousDistance = 0.0f;
	for (uint32_t i = 0; i < 4; i++)
	{
		const float p = (float)(i + 1) / 4.0f;
		const float v = nearPlane + range * p;
		const float d = LAMBDA * (nearPlane * powf(ratio, p) - v) + v;
		const float distance = (d - nearPlane) / range;

		vec4 frustumCorners[8];
		memcpy(frustumCorners, projectedCorners, sizeof(projectedCorners));

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

		FrustumCullingData *frustum = &frustums[i + 1];
		glm_mat4_copy(viewMatrix, frustum->viewMatrix);
		frustum->nearPlane = -FLT_MAX;
		frustum->farPlane = radius * 2;
		frustum->frustumPlanes[0] = frustumX[0];
		frustum->frustumPlanes[1] = frustumX[2];
		frustum->frustumPlanes[2] = frustumY[1];
		frustum->frustumPlanes[3] = frustumY[2];

		previousDistance = distance;
	}

	const LunaBufferWriteInfo depthsWriteInfo = {
		.bytes = sizeof(float) * 4,
		.data = depths,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.uniforms.lights, &depthsWriteInfo),
						   "Failed to write directional light cascade depths to buffer!");
	const LunaBufferWriteInfo matricesWriteInfo = {
		.bytes = sizeof(mat4) * 4,
		.data = matrices,
		.offset = sizeof(float) * 4,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.uniforms.lights, &matricesWriteInfo),
						   "Failed to write directional light cascade transform matrices to buffer!");
	const LunaBufferWriteInfo frustumsWriteInfo = {
		.bytes = sizeof(FrustumCullingData) * 4,
		.data = &frustums[1],
		.offset = sizeof(FrustumCullingData),
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.frustums, &frustumsWriteInfo),
						   "Failed to write directional light frustums to buffer!");

	return VK_SUCCESS;
}

VkResult WriteFrustumsBuffer()
{
	const LunaBufferWriteInfo frustumBufferWriteInfo = {
		.bytes = sizeof(FrustumCullingData) * frustumCount,
		.data = frustums,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
	};
	VulkanTestReturnResult(lunaWriteDataToBuffer(device, commandBuffer, buffers.frustums, &frustumBufferWriteInfo),
						   "Failed to write frustums buffer!");

	return VK_SUCCESS;
}

static inline bool OverlapsCameraFrustum(const Camera *camera, const FrustumCullingData *frustum) {}

VkResult CullLights()
{
	uint32_t frustumIndex = GetState()->map->directionalLight == NULL ? 1 : 5;
	for (uint32_t i = 0; i < GetState()->map->lightCount; i++)
	{
		const Light *light = &GetState()->map->lights[i];
		// switch (light->type)
		// {
		// 	case LIGHT_TYPE_SPOT:
		// 	{
		// 		glm_mat4_copy(viewMatrix, frustums[frustumIndex].viewMatrix);
		// 		frustums[frustumIndex].nearPlane = LIGHT_NEAR_PLANE;
		// 		frustums[frustumIndex].farPlane = light->maxDistance;
		// 		frustums[frustumIndex].frustumPlanes[0] = frustumX[0];
		// 		frustums[frustumIndex].frustumPlanes[1] = frustumX[2];
		// 		frustums[frustumIndex].frustumPlanes[2] = frustumY[1];
		// 		frustums[frustumIndex].frustumPlanes[3] = frustumY[2];
		// 		frustumIndex++;
		// 	}
		// 	break;
		// 	case LIGHT_TYPE_POINT:
		// 	{
		// 		light->shadowMapIndex = pointLightIndex++;
		// 		glm_perspective_lh_zo(glm_rad(90), 1, light->maxDistance, LIGHT_NEAR_PLANE, transformMatrix);
		//
		// 		mat4 transposed;
		// 		glm_mat4_transpose_to(transformMatrix, transposed);
		// 		vec4 frustumX;
		// 		vec4 frustumY;
		// 		glm_vec4_add(transposed[3], transposed[0], frustumX);
		// 		glm_vec4_add(transposed[3], transposed[1], frustumY);
		// 		glm_plane_normalize(frustumX);
		// 		glm_plane_normalize(frustumY);
		//
		// 		mat3 transforms[6] = {
		// 			{{0, 0, -1}, {0, 1, 0}, {1, 0, 0}},
		// 			{{0, 0, 1}, {0, 1, 0}, {-1, 0, 0}},
		// 			{{-1, 0, 0}, {0, 0, -1}, {0, -1, 0}},
		// 			{{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}},
		// 			{{-1, 0, 0}, {0, 1, 0}, {0, 0, -1}},
		// 			{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
		// 		};
		// 		vec3 negativeLightPosition = {
		// 			-light->transform.position.x,
		// 			-light->transform.position.y,
		// 			-light->transform.position.z,
		// 		};
		// 		for (uint32_t j = 0; j < 6; j++)
		// 		{
		// 			glm_mat4_ins3(transforms[j], frustums[frustumIndex].viewMatrix);
		// 			glm_translate(frustums[frustumIndex].viewMatrix, negativeLightPosition);
		// 			frustums[frustumIndex].nearPlane = LIGHT_NEAR_PLANE;
		// 			frustums[frustumIndex].farPlane = light->maxDistance;
		// 			frustums[frustumIndex].frustumPlanes[0] = frustumX[0];
		// 			frustums[frustumIndex].frustumPlanes[1] = frustumX[2];
		// 			frustums[frustumIndex].frustumPlanes[2] = frustumY[1];
		// 			frustums[frustumIndex].frustumPlanes[3] = frustumY[2];
		// 			frustumIndex++;
		// 		}
		// 	}
		// 	break;
		// 	default:
		// 		continue;
		// }
		//
		// // The allocation for lights is not aligned so we just memcpy
		// memcpy(light->transformMatrix, transformMatrix, sizeof(mat4));
	}
}

VkResult CullModels()
{
	VulkanTestReturnResult(UpdateDirectionalLightCascades(GetState()->camera, GetState()->map->directionalLight),
						   "Failed to update directional light cascades!");

	const LunaMultiBufferMemoryBarrier preClearMemoryBarrier = {
		.sourceStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
		.sourceAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		.destinationAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
		.bufferCount = perFrustumBuffersHandles.length,
		.buffers = (LunaBuffer *)perFrustumBuffersHandles.data->pointerData,
	};
	const LunaDependencyInfo preClearDependencyInfo = {
		.multiBufferMemoryBarrierCount = 1,
		.multiBufferMemoryBarriers = &preClearMemoryBarrier,
	};
	VulkanTestReturnResult(lunaPipelineBarrier(device, commandBuffer, &preClearDependencyInfo),
						   "Failed to insert pipeline barrier before clearing culling data!");

	VulkanTestReturnResult(lunaPushConstantsCompute(device, commandBuffer, pipelines.clearCullingData),
						   "Failed to push constants for clearing culling data!");
	const LunaDescriptorSetBindInfo clearDescriptorSetBindInfo = {
		.descriptorSetCount = 1,
		.descriptorSets = &descriptorSets.culling.set,
	};
	const LunaDispatchInfo clearDispatchInfo = {
		.pipeline = pipelines.clearCullingData,
		.descriptorSetBindInfo = &clearDescriptorSetBindInfo,
		.groupCountX = ((actorModelsDrawInfoCount + 63) / 64),
		.groupCountY = frustumCount,
	};
	VulkanTestReturnResult(lunaDispatch(device, commandBuffer, &clearDispatchInfo),
						   "Failed to dispatch culling data clear shader!");

	const LunaMultiBufferMemoryBarrier preDispatchMemoryBarrier = {
		.sourceStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		.sourceAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		.destinationAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
		.bufferCount = perFrustumBuffersHandles.length,
		.buffers = (LunaBuffer *)perFrustumBuffersHandles.data->pointerData,
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
		.groupCountX = ((maximumCulledInstanceCount + 63) / 64),
		.groupCountY = 8,
		.groupCountZ = frustumCount,
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
		.bufferCount = perFrustumBuffersHandles.length,
		.buffers = (LunaBuffer *)perFrustumBuffersHandles.data->pointerData,
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
