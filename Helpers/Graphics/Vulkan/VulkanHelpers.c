//
// Created by Noah on 11/23/2024.
//

#include "VulkanHelpers.h"
#include <assert.h>
#include <cglm/clipspace/persp_lh_zo.h>
#include <cglm/clipspace/view_lh_zo.h>
#include <luna/luna.h>
#include "../../CommonAssets.h"
#include "../../Core/Error.h"
#include "VulkanResources.h"

#pragma region variables
SDL_Window *vulkanWindow = NULL;
bool minimized = false;
bool shouldDropFrame = false;
size_t loadedActors = 0;

VkSurfaceKHR surface = VK_NULL_HANDLE;
VkPhysicalDeviceLimits physicalDeviceLimits = {0};
VkExtent2D swapChainExtent = {0};
LunaRenderPass renderPass = LUNA_NULL_HANDLE;
LunaDescriptorSetLayout descriptorSetLayout = LUNA_NULL_HANDLE;
Pipelines pipelines = {
	.ui = LUNA_NULL_HANDLE,
	.viewModel = LUNA_NULL_HANDLE,
	.sky = LUNA_NULL_HANDLE,
	.floorAndCeiling = LUNA_NULL_HANDLE,
	.walls = LUNA_NULL_HANDLE,
	.actorWalls = LUNA_NULL_HANDLE,
	.shadedActorModels = LUNA_NULL_HANDLE,
	.unshadedActorModels = LUNA_NULL_HANDLE,
};
uint8_t currentFrame = 0;
Buffers buffers = {
	.ui.vertices.allocatedSize = sizeof(UiVertex) * 4 * MAX_UI_QUADS_INIT,
	.ui.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_UI_QUADS_INIT,
	.sky.vertices.allocatedSize = 0,
	.sky.indices.allocatedSize = 0,
	.walls.vertices.allocatedSize = sizeof(WallVertex) * 4 * MAX_WALLS_INIT,
	.walls.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_WALLS_INIT,
	.actorWalls.vertices.allocatedSize = sizeof(ActorVertex) * 4 * MAX_WALL_ACTORS_INIT,
	.actorWalls.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_WALL_ACTORS_INIT,
	.actorWalls.instanceData.allocatedSize = sizeof(ActorWallInstanceData) * MAX_WALL_ACTORS_INIT,
	.actorWalls.drawInfo.allocatedSize = sizeof(VkDrawIndexedIndirectCommand) * MAX_WALL_ACTORS_INIT,
	.actorModels.vertices.allocatedSize = sizeof(ModelVertex) * 4 * MAX_MODEL_ACTOR_QUADS_INIT,
	.actorModels.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_MODEL_ACTOR_QUADS_INIT,
	.actorModels.instanceData.allocatedSize = sizeof(ModelInstanceData) * MAX_MODEL_ACTOR_QUADS_INIT,
	.actorModels.shadedDrawInfo.allocatedSize = sizeof(VkDrawIndexedIndirectCommand) * MAX_MODEL_ACTOR_QUADS_INIT,
	.actorModels.unshadedDrawInfo.allocatedSize = sizeof(VkDrawIndexedIndirectCommand) * MAX_MODEL_ACTOR_QUADS_INIT,
};
LunaDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
List textures = {0};
uint32_t imageAssetIdToIndexMap[MAX_TEXTURES];
TextureSamplers textureSamplers = {
	.linearRepeat = LUNA_NULL_HANDLE,
	.nearestRepeat = LUNA_NULL_HANDLE,
	.linearNoRepeat = LUNA_NULL_HANDLE,
	.nearestNoRepeat = LUNA_NULL_HANDLE,
};
VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
PushConstants pushConstants = {0};
List loadedLodIds = {0};
List lodCounts = {0};
#pragma endregion variables


VkResult CreateShaderModule(const char *path, LunaShaderModule *shaderModule)
{
	const Asset *shader = DecompressAsset(path);
	if (!shader)
	{
		return VK_ERROR_UNKNOWN;
	}

	// sizeof(uint32_t) * 4 is the asset header
	VulkanTestReturnResult(lunaCreateShaderModule((uint32_t *)shader->data,
												  shader->size - sizeof(uint32_t) * 4,
												  shaderModule),
						   "Failed to create shader module!");

	return VK_SUCCESS;
}

inline uint32_t TextureIndex(const char *texture)
{
	return ImageIndex(LoadImage(texture));
}

inline uint32_t ImageIndex(const Image *image)
{
	const uint32_t index = imageAssetIdToIndexMap[image->id];
	if (index == -1)
	{
		if (!LoadTexture(image))
		{
			Error("Failed to load texture!");
		}
		return imageAssetIdToIndexMap[image->id];
	}
	return index;
}

VkResult LoadSky(const ModelDefinition *skyModel)
{
	buffers.sky.vertices.bytesUsed = sizeof(SkyVertex) * skyModel->lods[0]->vertexCount;
	buffers.sky.indices.bytesUsed = 0;
	for (uint32_t i = 0; i < skyModel->materialCount; i++)
	{
		buffers.sky.indices.bytesUsed += sizeof(uint32_t) * skyModel->lods[0]->indexCount[i];
	}
	if (buffers.sky.vertices.allocatedSize == 0 || buffers.sky.indices.allocatedSize == 0)
	{
		assert(buffers.sky.vertices.allocatedSize == 0 && buffers.sky.indices.allocatedSize == 0);
		const LunaBufferCreationInfo vertexBufferCreationInfo = {
			.size = buffers.sky.vertices.bytesUsed,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&vertexBufferCreationInfo, &buffers.sky.vertices.buffer),
							   "Failed to create sky vertex buffer!");
		buffers.sky.vertices.data = malloc(buffers.sky.vertices.bytesUsed);
		CheckAlloc(buffers.sky.vertices.data);
		buffers.sky.vertices.allocatedSize = buffers.sky.vertices.bytesUsed;

		const LunaBufferCreationInfo indexBufferCreationInfo = {
			.size = buffers.sky.indices.bytesUsed,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&indexBufferCreationInfo, &buffers.sky.indices.buffer),
							   "Failed to create sky index buffer!");
		buffers.sky.indices.data = malloc(buffers.sky.indices.bytesUsed);
		CheckAlloc(buffers.sky.indices.data);
		buffers.sky.indices.allocatedSize = buffers.sky.indices.bytesUsed;
	} else if (buffers.sky.vertices.allocatedSize < buffers.sky.vertices.bytesUsed ||
			   buffers.sky.indices.allocatedSize < buffers.sky.indices.bytesUsed)
	{
		lunaDestroyBuffer(buffers.sky.vertices.buffer);
		const LunaBufferCreationInfo vertexBufferCreationInfo = {
			.size = buffers.sky.vertices.bytesUsed,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&vertexBufferCreationInfo, &buffers.sky.vertices.buffer),
							   "Failed to recreate sky vertex buffer!");
		void *newVertices = realloc(buffers.sky.vertices.data, buffers.sky.vertices.bytesUsed);
		CheckAlloc(newVertices);
		buffers.sky.vertices.data = newVertices;
		buffers.sky.vertices.allocatedSize = buffers.sky.vertices.bytesUsed;

		lunaDestroyBuffer(buffers.sky.indices.buffer);
		const LunaBufferCreationInfo indexBufferCreationInfo = {
			.size = buffers.sky.indices.bytesUsed,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		};
		VulkanTestReturnResult(lunaCreateBuffer(&indexBufferCreationInfo, &buffers.sky.indices.buffer),
							   "Failed to recreate sky index buffer!");
		void *newIndices = realloc(buffers.sky.indices.data, buffers.sky.indices.bytesUsed);
		CheckAlloc(newIndices);
		buffers.sky.indices.data = newIndices;
		buffers.sky.indices.allocatedSize = buffers.sky.indices.bytesUsed;
	}


	SkyVertex *vertices = buffers.sky.vertices.data;
	uint32_t *indices = buffers.sky.indices.data;
	for (uint32_t i = 0; i < skyModel->lods[0]->vertexCount; i++)
	{
		// Copy {x, y, z, u, v} and discard {nx, ny, nz}
		memcpy(&vertices[i], skyModel->lods[0]->vertexData + i * 8, sizeof(float) * 5);
	}
	buffers.sky.objectCount = 0;
	for (uint32_t i = 0; i < skyModel->materialCount; i++)
	{
		memcpy(indices + buffers.sky.objectCount,
			   skyModel->lods[0]->indexData[i],
			   sizeof(uint32_t) * skyModel->lods[0]->indexCount[i]);
		buffers.sky.objectCount += skyModel->lods[0]->indexCount[i];
	}
	lunaWriteDataToBuffer(buffers.sky.vertices.buffer, vertices, buffers.sky.vertices.bytesUsed, 0);
	lunaWriteDataToBuffer(buffers.sky.indices.buffer, indices, buffers.sky.indices.bytesUsed, 0);

	return VK_SUCCESS;
}

void LoadWalls(const Level *level)
{
	WallVertex *vertices = buffers.walls.vertices.data;
	uint32_t *indices = buffers.walls.indices.data;

	for (uint32_t i = 0; i < buffers.walls.objectCount; i++)
	{
		const Wall *wall = ListGet(level->walls, i);
		const float halfHeight = wall->height / 2.0f;
		const vec2 startVertex = {(float)wall->a.x, (float)wall->a.y};
		const vec2 endVertex = {(float)wall->b.x, (float)wall->b.y};
		const vec2 startUV = {wall->uvOffset, 0};
		const vec2 endUV = {(float)(wall->uvScale * wall->length + wall->uvOffset), 1};

		vertices[4 * i].x = startVertex[0];
		vertices[4 * i].y = halfHeight;
		vertices[4 * i].z = startVertex[1];
		vertices[4 * i].u = startUV[0];
		vertices[4 * i].v = startUV[1];
		vertices[4 * i].textureIndex = TextureIndex(wall->tex);
		vertices[4 * i].wallAngle = (float)wall->angle;

		vertices[4 * i + 1].x = endVertex[0];
		vertices[4 * i + 1].y = halfHeight;
		vertices[4 * i + 1].z = endVertex[1];
		vertices[4 * i + 1].u = endUV[0];
		vertices[4 * i + 1].v = startUV[1];
		vertices[4 * i + 1].textureIndex = TextureIndex(wall->tex);
		vertices[4 * i + 1].wallAngle = (float)wall->angle;

		vertices[4 * i + 2].x = endVertex[0];
		vertices[4 * i + 2].y = -halfHeight;
		vertices[4 * i + 2].z = endVertex[1];
		vertices[4 * i + 2].u = endUV[0];
		vertices[4 * i + 2].v = endUV[1];
		vertices[4 * i + 2].textureIndex = TextureIndex(wall->tex);
		vertices[4 * i + 2].wallAngle = (float)wall->angle;

		vertices[4 * i + 3].x = startVertex[0];
		vertices[4 * i + 3].y = -halfHeight;
		vertices[4 * i + 3].z = startVertex[1];
		vertices[4 * i + 3].u = startUV[0];
		vertices[4 * i + 3].v = endUV[1];
		vertices[4 * i + 3].textureIndex = TextureIndex(wall->tex);
		vertices[4 * i + 3].wallAngle = (float)wall->angle;

		indices[6 * i] = i * 4;
		indices[6 * i + 1] = i * 4 + 1;
		indices[6 * i + 2] = i * 4 + 2;
		indices[6 * i + 3] = i * 4;
		indices[6 * i + 4] = i * 4 + 2;
		indices[6 * i + 5] = i * 4 + 3;
	}
	lunaWriteDataToBuffer(buffers.walls.vertices.buffer, vertices, buffers.walls.vertices.bytesUsed, 0);
	lunaWriteDataToBuffer(buffers.walls.indices.buffer, indices, buffers.walls.indices.bytesUsed, 0);
}

void UpdateTranslationMatrix(const Camera *camera)
{
	mat4 perspectiveMatrix;
	glm_perspective_lh_zo(glm_rad(camera->fov),
						  (float)swapChainExtent.width / (float)swapChainExtent.height,
						  NEAR_Z,
						  FAR_Z,
						  perspectiveMatrix);

	vec3 viewTarget = {cosf(camera->yaw), 0, sinf(camera->yaw)};

	// TODO roll and pitch might be messed up (test and fix as needed)
	glm_vec3_rotate(viewTarget, camera->roll, GLM_ZUP); // Roll
	glm_vec3_rotate(viewTarget, camera->pitch, GLM_XUP); // Pitch

	vec3 cameraPosition = {camera->x, camera->y, camera->z};
	glm_vec3_add(viewTarget, cameraPosition, viewTarget);

	mat4 viewMatrix;
	glm_lookat_lh_zo(cameraPosition, viewTarget, (vec3){0.0f, -1.0f, 0.0f}, viewMatrix);

	glm_mat4_mul(perspectiveMatrix, viewMatrix, pushConstants.translationMatrix);
}

void UpdateViewModelMatrix(const Viewmodel *viewmodel)
{
	mat4 perspectiveMatrix;
	glm_perspective_lh_zo(glm_rad(VIEWMODEL_FOV),
						  (float)swapChainExtent.width / (float)swapChainExtent.height,
						  NEAR_Z,
						  FAR_Z,
						  perspectiveMatrix);

	mat4 translationMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_translate(translationMatrix,
				  (vec3){viewmodel->translation[0], -viewmodel->translation[1], viewmodel->translation[2]});

	// TODO rotation other than yaw
	mat4 rotationMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_rotate(rotationMatrix, viewmodel->rotation[1], (vec3){0.0f, -1.0f, 0.0f});

	mat4 viewModelMatrix;
	glm_mat4_mul(translationMatrix, rotationMatrix, translationMatrix);
	glm_mat4_mul(perspectiveMatrix, translationMatrix, viewModelMatrix);

	for (uint32_t i = 0; i < buffers.viewModel.drawCount; i++)
	{
		memcpy(buffers.viewModel.instanceDatas[i].transform, viewModelMatrix, sizeof(mat4));
	}
	lunaWriteDataToBuffer(buffers.viewModel.instanceDataBuffer,
						  buffers.viewModel.instanceDatas,
						  sizeof(ModelInstanceData) * buffers.viewModel.drawCount,
						  0);
}

void DrawRectInternal(const float ndcStartX,
					  const float ndcStartY,
					  const float ndcEndX,
					  const float ndcEndY,
					  const float startU,
					  const float startV,
					  const float endU,
					  const float endV,
					  const Color color,
					  const uint32_t textureIndex)
{
	const mat4 matrix = {
		{ndcEndX, ndcStartY, endU, startV},
		{ndcStartX, ndcStartY, startU, startV},
		{ndcStartX, ndcEndY, startU, endV},
		{ndcEndX, ndcEndY, endU, endV},
	};
	DrawQuadInternal(matrix, color, textureIndex);
}

void DrawQuadInternal(const mat4 vertices_posXY_uvZW, const Color color, const uint32_t textureIndex)
{
	if (buffers.ui.vertices.allocatedSize < buffers.ui.vertices.bytesUsed + sizeof(UiVertex) * 4 ||
		buffers.ui.indices.allocatedSize < buffers.ui.indices.bytesUsed + sizeof(uint32_t) * 6)
	{
		buffers.ui.vertices.allocatedSize += sizeof(UiVertex) * 4 * 16;
		buffers.ui.indices.allocatedSize += sizeof(uint32_t) * 6 * 16;
		buffers.ui.shouldResize = true;

		UiVertex *newVertices = realloc(buffers.ui.vertices.data, buffers.ui.vertices.allocatedSize);
		CheckAlloc(newVertices);
		buffers.ui.vertices.data = newVertices;

		uint32_t *newIndices = realloc(buffers.ui.indices.data, buffers.ui.indices.allocatedSize);
		CheckAlloc(newIndices);
		buffers.ui.indices.data = newIndices;
	}

	uint32_t *indices = buffers.ui.indices.data;
	UiVertex *vertices = buffers.ui.vertices.data;

	vertices[4 * buffers.ui.objectCount] = (UiVertex){
		.x = vertices_posXY_uvZW[0][0],
		.y = vertices_posXY_uvZW[0][1],
		.u = vertices_posXY_uvZW[0][2],
		.v = vertices_posXY_uvZW[0][3],
		.r = color.r,
		.g = color.g,
		.b = color.b,
		.a = color.a,
		.textureIndex = textureIndex,
	};
	vertices[4 * buffers.ui.objectCount + 1] = (UiVertex){
		.x = vertices_posXY_uvZW[1][0],
		.y = vertices_posXY_uvZW[1][1],
		.u = vertices_posXY_uvZW[1][2],
		.v = vertices_posXY_uvZW[1][3],
		.r = color.r,
		.g = color.g,
		.b = color.b,
		.a = color.a,
		.textureIndex = textureIndex,
	};
	vertices[4 * buffers.ui.objectCount + 2] = (UiVertex){
		.x = vertices_posXY_uvZW[2][0],
		.y = vertices_posXY_uvZW[2][1],
		.u = vertices_posXY_uvZW[2][2],
		.v = vertices_posXY_uvZW[2][3],
		.r = color.r,
		.g = color.g,
		.b = color.b,
		.a = color.a,
		.textureIndex = textureIndex,
	};
	vertices[4 * buffers.ui.objectCount + 3] = (UiVertex){
		.x = vertices_posXY_uvZW[3][0],
		.y = vertices_posXY_uvZW[3][1],
		.u = vertices_posXY_uvZW[3][2],
		.v = vertices_posXY_uvZW[3][3],
		.r = color.r,
		.g = color.g,
		.b = color.b,
		.a = color.a,
		.textureIndex = textureIndex,
	};

	indices[6 * buffers.ui.objectCount] = buffers.ui.objectCount * 4;
	indices[6 * buffers.ui.objectCount + 1] = buffers.ui.objectCount * 4 + 1;
	indices[6 * buffers.ui.objectCount + 2] = buffers.ui.objectCount * 4 + 2;
	indices[6 * buffers.ui.objectCount + 3] = buffers.ui.objectCount * 4;
	indices[6 * buffers.ui.objectCount + 4] = buffers.ui.objectCount * 4 + 2;
	indices[6 * buffers.ui.objectCount + 5] = buffers.ui.objectCount * 4 + 3;

	buffers.ui.objectCount++;
	buffers.ui.vertices.bytesUsed += sizeof(UiVertex) * 4;
	buffers.ui.indices.bytesUsed += sizeof(uint32_t) * 6;
}
