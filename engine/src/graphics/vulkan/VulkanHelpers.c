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
#include <engine/graphics/vulkan/VulkanResources.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/List.h>
#include <engine/structs/Viewmodel.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <luna/luna.h>
#include <luna/lunaImage.h>
#include <luna/lunaTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "engine/graphics/vulkan/VulkanInternal.h"

#pragma region variables
bool minimized = false;
LunaDevice device = LUNA_NULL_HANDLE;
uint32_t queueFamilyIndex = -1u;
VkQueue queue = VK_NULL_HANDLE;
LunaCommandPool commandPool = LUNA_NULL_HANDLE;
LunaCommandBuffer commandBuffer = LUNA_NULL_HANDLE;
LunaCommandBuffer secondaryCommandBuffer = LUNA_NULL_HANDLE;
LunaSemaphore semaphore = LUNA_NULL_HANDLE;
VkSurfaceKHR surface = VK_NULL_HANDLE;
VkExtent2D swapChainExtent = {0};
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
Buffers buffers = {
#ifdef JPH_DEBUG_RENDERER
	.debugDrawLines.vertices.allocatedSize = sizeof(DebugDrawVertex) * MAX_DEBUG_DRAW_VERTICES_INIT,
	.debugDrawTriangles.vertices.allocatedSize = sizeof(DebugDrawVertex) * MAX_DEBUG_DRAW_VERTICES_INIT,
#endif
};
Pipelines pipelines = {
	.ui = LUNA_NULL_HANDLE,
#ifdef JPH_DEBUG_RENDERER
	.debugDrawLines = LUNA_NULL_HANDLE,
	.debugDrawTriangles = LUNA_NULL_HANDLE,
#endif
};
uint32_t pendingTasks = 0;
uint32_t skyTextureIndex = 0;
#pragma endregion variables

bool ClearTextureCache()
{
	memset(imageAssetIdToIndexMap, -1, sizeof(*imageAssetIdToIndexMap) * MAX_TEXTURES);
	for (size_t i = 0; i < textures.length; i++)
	{
		lunaDestroyImage(device, (LunaImage)ListGetPointer(textures, i));
	}
	ListClear(textures);
	lunaDestroySampler(device, textureSamplers.linearRepeatAnisotropy);
	lunaDestroySampler(device, textureSamplers.linearNoRepeatAnisotropy);
	lunaDestroySampler(device, textureSamplers.linearRepeatNoAnisotropy);
	lunaDestroySampler(device, textureSamplers.nearestRepeatNoAnisotropy);
	lunaDestroySampler(device, textureSamplers.linearNoRepeatNoAnisotropy);
	lunaDestroySampler(device, textureSamplers.nearestNoRepeatNoAnisotropy);
	return CreateTextureSamplers();
}

void ClearModelCache()
{
	// TODO: Implement me
}

VkResult CreateShaderModule(const char *path, const ShaderType shaderType, LunaShaderModule *shaderModule)
{
	Shader *shader = LoadShader(path);
	if (!shader)
	{
		return VK_ERROR_UNKNOWN;
	}
	assert(shader->platform == PLATFORM_VULKAN);
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
			// TODO: If loading a texture fails it can't fall back to OpenGL.
			//  There is no easy way to fix this with the current system, since the return value of this function is not
			//  checked but instead is just assumed to be valid. That rules out returning something like -1 on error.
			Error("Failed to load texture!");
		}
		return imageAssetIdToIndexMap[image->id];
	}
	return index;
}

// TODO: Make sure this doesn't need changes
VkResult UpdateCameraUniform(const Camera *camera)
{
	mat4 perspectiveMatrix;
	glm_perspective_lh_zo(glm_rad(camera->fov),
						  (float)swapChainExtent.width / (float)swapChainExtent.height,
						  NEAR_Z,
						  FAR_Z,
						  perspectiveMatrix);

	versor rotationQuat;
	QUAT_TO_VERSOR(camera->transform.rotation, rotationQuat);
	versor rotationOffset;
	glm_quatv(rotationOffset, GLM_PIf, GLM_XUP);
	glm_quat_mul(rotationQuat, rotationOffset, rotationQuat);

	vec3 cameraPosition = {camera->transform.position.x, camera->transform.position.y, camera->transform.position.z};
	mat4 viewMatrix;
	glm_quat_look(cameraPosition, rotationQuat, viewMatrix);

	CameraUniform uniform;
	glm_mat4_mul(perspectiveMatrix, viewMatrix, uniform.transform);
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

// TODO: Update this
VkResult UpdateViewModelMatrix(const Viewmodel *viewmodel)
{
	mat4 perspectiveMatrix;
	glm_perspective_lh_zo(glm_rad(VIEWMODEL_FOV),
						  (float)swapChainExtent.width / (float)swapChainExtent.height,
						  NEAR_Z,
						  FAR_Z,
						  perspectiveMatrix);

	mat4 translationMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_translate(translationMatrix,
				  (vec3){viewmodel->transform.position.x,
						 -viewmodel->transform.position.y,
						 viewmodel->transform.position.z});

	mat4 rotationMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_rotate(rotationMatrix,
			   JPH_Quat_GetRotationAngle(&viewmodel->transform.rotation, &Vector3_AxisY),
			   (vec3){0.0f, -1.0f, 0.0f});

	mat4 viewModelMatrix;
	glm_mat4_mul(translationMatrix, rotationMatrix, translationMatrix);
	glm_mat4_mul(perspectiveMatrix, translationMatrix, viewModelMatrix);

	const size_t instanceCount = lunaGetBufferSize(buffers.viewmodel.instanceData) / sizeof(ModelInstanceData);
	for (size_t i = 0; i < instanceCount; i++)
	{
		const LunaBufferWriteInfo writeInfo = {
			.bytes = sizeof(mat4),
			.data = viewModelMatrix,
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
