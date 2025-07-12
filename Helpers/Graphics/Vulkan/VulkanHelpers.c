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
	.walls = LUNA_NULL_HANDLE,
	.actorWalls = LUNA_NULL_HANDLE,
	.shadedActorModels = LUNA_NULL_HANDLE,
	.unshadedActorModels = LUNA_NULL_HANDLE,
	.ui = LUNA_NULL_HANDLE,
};
uint8_t currentFrame = 0;
Buffers buffers = {
	.ui.vertices.allocatedSize = sizeof(UiVertex) * 4 * MAX_UI_QUADS_INIT,
	.ui.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_UI_QUADS_INIT,
	.walls.vertices.allocatedSize = sizeof(WallVertex) * 4 * MAX_WALLS_INIT,
	.walls.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_WALLS_INIT,
	.actorWalls.vertices.allocatedSize = sizeof(ActorVertex) * 4 * MAX_WALL_ACTORS_INIT,
	.actorWalls.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_WALL_ACTORS_INIT,
	.actorWalls.instanceData.allocatedSize = sizeof(ActorWallInstanceData) * MAX_WALL_ACTORS_INIT,
	.actorWalls.drawInfo.allocatedSize = sizeof(VkDrawIndexedIndirectCommand) * MAX_WALL_ACTORS_INIT,
	.actorModels.vertices.allocatedSize = sizeof(ActorModelVertex) * 4 * MAX_MODEL_ACTOR_QUADS_INIT,
	.actorModels.indices.allocatedSize = sizeof(uint32_t) * 6 * MAX_MODEL_ACTOR_QUADS_INIT,
	.actorModels.instanceData.allocatedSize = sizeof(ActorModelInstanceData) * MAX_MODEL_ACTOR_QUADS_INIT,
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

void LoadRoof(const bool hasCeiling, const uint32_t ceilingTextureIndex)
{
	WallVertex *vertices = buffers.roof.vertices.data;
	uint32_t *indices = buffers.roof.indices.data;
	if (hasCeiling)
	{
		vertices[0].x = -100;
		vertices[0].y = 0.5f;
		vertices[0].z = -100;
		vertices[0].u = -100;
		vertices[0].v = -100;
		vertices[0].textureIndex = ceilingTextureIndex;

		vertices[1].x = 100;
		vertices[1].y = 0.5f;
		vertices[1].z = -100;
		vertices[1].u = 100;
		vertices[1].v = -100;
		vertices[1].textureIndex = ceilingTextureIndex;

		vertices[2].x = 100;
		vertices[2].y = 0.5f;
		vertices[2].z = 100;
		vertices[2].u = 100;
		vertices[2].v = 100;
		vertices[2].textureIndex = ceilingTextureIndex;

		vertices[3].x = -100;
		vertices[3].y = 0.5f;
		vertices[3].z = 100;
		vertices[3].u = -100;
		vertices[3].v = 100;
		vertices[3].textureIndex = ceilingTextureIndex;

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 0;
		indices[4] = 2;
		indices[5] = 3;
		buffers.roof.indexCount = 6;
	} else
	{
		for (uint32_t i = 0; i < pushConstants.skyVertexCount; i++)
		{
			// Copy {x, y, z, u, v} and discard {nx, ny, nz}
			memcpy(&vertices[i], &skyModel->lods[0]->vertexData[i * 8], sizeof(float) * 5);
			vertices[i].textureIndex = pushConstants.skyTextureIndex;
		}
		buffers.roof.indexCount = 0;
		for (uint32_t i = 0; i < skyModel->materialCount; i++)
		{
			const size_t indexCount = skyModel->lods[0]->indexCount[i];
			memcpy(indices + buffers.roof.indexCount, skyModel->lods[0]->indexData[i], sizeof(uint32_t) * indexCount);
			buffers.roof.indexCount += indexCount;
		}
	}
	lunaWriteDataToBuffer(buffers.roof.vertices.buffer, vertices, buffers.roof.vertices.allocatedSize, 0);
	lunaWriteDataToBuffer(buffers.roof.indices.buffer, indices, buffers.roof.indices.allocatedSize, 0);
}

void LoadWalls(const Level *level)
{
	WallVertex *vertices = buffers.walls.vertices.data;
	uint32_t *indices = buffers.walls.indices.data;

	vertices[0].x = -100;
	vertices[0].y = -0.5f;
	vertices[0].z = -100;
	vertices[0].u = -100;
	vertices[0].v = -100;
	vertices[0].textureIndex = TextureIndex(level->floorTex);

	vertices[1].x = 100;
	vertices[1].y = -0.5f;
	vertices[1].z = -100;
	vertices[1].u = 100;
	vertices[1].v = -100;
	vertices[1].textureIndex = TextureIndex(level->floorTex);

	vertices[2].x = 100;
	vertices[2].y = -0.5f;
	vertices[2].z = 100;
	vertices[2].u = 100;
	vertices[2].v = 100;
	vertices[2].textureIndex = TextureIndex(level->floorTex);

	vertices[3].x = -100;
	vertices[3].y = -0.5f;
	vertices[3].z = 100;
	vertices[3].u = -100;
	vertices[3].v = 100;
	vertices[3].textureIndex = TextureIndex(level->floorTex);

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;

	for (uint32_t i = 1; i < buffers.walls.objectCount; i++)
	{
		const Wall *wall = ListGet(level->walls, i - 1);
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
	mat4 perspective;
	glm_perspective_lh_zo(glm_rad(camera->fov),
						  (float)swapChainExtent.width / (float)swapChainExtent.height,
						  NEAR_Z,
						  FAR_Z,
						  perspective);

	vec3 viewTarget = {cosf(camera->yaw), 0, sinf(camera->yaw)};

	// TODO roll and pitch might be messed up (test and fix as needed)
	glm_vec3_rotate(viewTarget, camera->roll, GLM_ZUP); // Roll
	glm_vec3_rotate(viewTarget, camera->pitch, GLM_XUP); // Pitch

	vec3 cameraPosition = {camera->x, camera->y, camera->z};
	glm_vec3_add(viewTarget, cameraPosition, viewTarget);

	mat4 view;
	glm_lookat_lh_zo(cameraPosition, viewTarget, (vec3){0.0f, -1.0f, 0.0f}, view);

	glm_mat4_mul(perspective, view, pushConstants.translationMatrix);
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
		{ndcStartX, ndcStartY, startU, startV},
		{ndcEndX, ndcStartY, endU, startV},
		{ndcEndX, ndcEndY, endU, endV},
		{ndcStartX, ndcEndY, startU, endV},
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
