//
// Created by Noah on 11/23/2024.
//

#ifndef VULKANHELPERS_H
#define VULKANHELPERS_H

#include <cglm/types.h>
#include <engine/assets/ModelLoader.h>
#include <engine/assets/ShaderLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Viewmodel.h>
#include <engine/subsystem/Logging.h>
#include <joltc/Math/Vector3.h>
#include <luna/lunaTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#pragma region macros
#define MAX_FRAMES_IN_FLIGHT 1
#define MAX_UI_QUADS_INIT 8192 // TODO: find best value (and a fix for resizing the buffer mid-frame)
#define MAX_WALLS_INIT 1024
#define MAX_WALL_ACTORS_INIT 1
#define MAX_MODEL_ACTOR_QUADS_INIT 1
#define MAX_DEBUG_DRAW_VERTICES_INIT 1024

#define VulkanLogError(...) LogInternal("VULKAN", 31, true, __VA_ARGS__)
// TODO Use LogInternal
#define VulkanTestInternal(function, returnValue, ...) \
	{ \
		const VkResult result = function; \
		if (result != VK_SUCCESS) \
		{ \
			LogInternal("VULKAN", 31, false, __VA_ARGS__); \
			LogInternal(NULL, 0, true, "Error code: %d\n", result); \
			if (result == VK_ERROR_DEVICE_LOST) \
			{ \
				LogInfo("See https://starflight.dev/media/VK_ERROR_DEVICE_LOST.webp for more information\n"); \
			} \
			return returnValue; \
		} \
	}
#define VulkanTestReturnResult(function, ...) VulkanTestInternal(function, result, __VA_ARGS__)
#define VulkanTest(function, ...) VulkanTestInternal(function, false, __VA_ARGS__)
#define VulkanTestResizeSwapchain(function, ...) \
	{ \
		const VkResult resizeCheckResult = function; \
		if (resizeCheckResult != VK_SUCCESS) \
		{ \
			if (resizeCheckResult == VK_ERROR_OUT_OF_DATE_KHR || resizeCheckResult == VK_SUBOPTIMAL_KHR) \
			{ \
				const LunaRenderPassResizeInfo renderPassResizeInfo = { \
					.renderPass = renderPass, \
					.width = LUNA_RENDER_PASS_WIDTH_SWAPCHAIN_WIDTH, \
					.height = LUNA_RENDER_PASS_HEIGHT_SWAPCHAIN_HEIGHT, \
				}; \
				VulkanTestReturnResult(lunaResizeSwapchain(1, &renderPassResizeInfo, NULL, &swapChainExtent), \
									   "Failed to resize swapchain!"); \
				UnlockLodThreadMutex(); \
				return resizeCheckResult; \
			} \
			VulkanTestReturnResult(resizeCheckResult, __VA_ARGS__); \
		} \
	}
#pragma endregion macros

#pragma region typedefs
enum VendorIDs
{
	AMD = 0x1002,
	APPLE = 0x106B,
	ARM = 0x13B5,
	IMG_TEC = 0x1010,
	INTEL = 0x8086,
	MESA = VK_VENDOR_ID_MESA,
	MICROSOFT = 0x1414,
	NVIDIA = 0x10DE,
	QUALCOMM = 0x5143,
};

typedef struct UiVertex
{
	float x;
	float y;

	float u;
	float v;

	float r;
	float g;
	float b;
	float a;

	uint32_t textureIndex;
} UiVertex;

typedef struct ModelVertex
{
	/// The position of the vertex in local space
	Vector3 position;

	/// The u component of the vertex's uv
	float u;
	/// The v component of the vertex's uv
	float v;

	/// The color of the vertex
	Color color;

	/// The normal of the vertex
	Vector3 normal;
} ModelVertex;

typedef struct ModelInstanceData
{
	/// The instance's transformation matrix.
	mat4 transform;
	/// The instance's texture index.
	uint32_t textureIndex;
	/// The color of the instance, given by the material
	Color materialColor;
	/// The color of the instance, specified per-instance (such as by Actor::modColor)
	Color instanceColor;
} ModelInstanceData;

typedef struct SkyVertex
{
	Vector3 position;

	float u;
	float v;
} SkyVertex;

typedef struct WallVertex
{
	Vector3 position;

	float u;
	float v;

	uint32_t textureIndex; // TODO Per-vertex is less than ideal
	float wallAngle;
} WallVertex;

typedef struct ActorWallVertex
{
	/// The position of the vertex in local space
	Vector3 position;

	/// The u component of the vertex's uv
	float u;
	/// The v component of the vertex's uv
	float v;
} ActorWallVertex;

typedef struct ActorWallInstanceData
{
	/// The instance's transformation matrix.
	mat4 transform;
	/// The instance's texture index.
	uint32_t textureIndex;
	/// The instance's rotation.
	float wallAngle;
} ActorWallInstanceData;

typedef struct DebugDrawVertex
{
	Vector3 position;
	Color color;
} DebugDrawVertex;

typedef struct BufferRegion
{
	LunaBuffer buffer;
	VkDeviceSize bytesUsed;
	VkDeviceSize allocatedSize;
	void *data;
} BufferRegion;

typedef struct UiBuffer
{
	BufferRegion vertices;
	BufferRegion indices;
	bool shouldResize;
} UiBuffer;

typedef struct ViewModelBuffer
{
	LunaBuffer vertices;
	LunaBuffer indices;
	LunaBuffer instanceDataBuffer;
	ModelInstanceData *instanceDatas;
	LunaBuffer drawInfo;
	uint32_t drawCount;
} ViewModelBuffer;

typedef struct SkyBuffer
{
	BufferRegion vertices;
	BufferRegion indices;
	uint32_t indexCount;
} SkyBuffer;

typedef struct WallsBuffer
{
	BufferRegion vertices;
	BufferRegion indices;
} WallsBuffer;

typedef struct ActorWallsBuffer
{
	BufferRegion vertices;
	BufferRegion indices;
	BufferRegion instanceData;
	BufferRegion drawInfo;
	VkDeviceSize count;
} ActorWallsBuffer;

typedef struct ActorModelsBuffer
{
	BufferRegion vertices;
	BufferRegion indices;
	BufferRegion instanceData;
	BufferRegion shadedDrawInfo;
	BufferRegion unshadedDrawInfo;

	List loadedModelIds;
} ActorModelsBuffer;

typedef struct DebugDrawBuffer
{
	BufferRegion vertices;
	uint32_t vertexCount;
	bool shouldResize;
} DebugDrawBuffer;

typedef struct Buffers
{
	UiBuffer ui;
	ViewModelBuffer viewModel;
	SkyBuffer sky;
	WallsBuffer walls;
	ActorWallsBuffer actorWalls;
	ActorModelsBuffer actorModels;
#ifdef JPH_DEBUG_RENDERER
	DebugDrawBuffer debugDrawLines;
	DebugDrawBuffer debugDrawTriangles;
#endif
} Buffers;

typedef struct Pipelines
{
	LunaGraphicsPipeline ui;
	LunaGraphicsPipeline viewModel;
	LunaGraphicsPipeline sky;
	LunaGraphicsPipeline floorAndCeiling;
	LunaGraphicsPipeline walls;
	LunaGraphicsPipeline actorWalls;
	LunaGraphicsPipeline shadedActorModels;
	LunaGraphicsPipeline unshadedActorModels;
#ifdef JPH_DEBUG_RENDERER
	LunaGraphicsPipeline debugDrawLines;
	LunaGraphicsPipeline debugDrawTriangles;
#endif
} Pipelines;

typedef struct TextureSamplers
{
	LunaSampler linearRepeat;
	LunaSampler nearestRepeat;
	LunaSampler linearNoRepeat;
	LunaSampler nearestNoRepeat;
} TextureSamplers;

typedef struct __attribute__((aligned(16))) PushConstants
{
	mat4 transformMatrix;
	Color fogColor;
	Vector3 cameraPosition;
	uint32_t roofTextureIndex;
	uint32_t floorTextureIndex;
	float yaw;
	float fogStart;
	float fogEnd;
} PushConstants;
#pragma endregion typedefs

#pragma region variables
extern bool minimized;
extern VkExtent2D swapChainExtent;
extern VkSampleCountFlagBits msaaSamples;
extern LockingList textures;
extern uint32_t imageAssetIdToIndexMap[MAX_TEXTURES];
extern LunaDescriptorSetLayout descriptorSetLayout;
extern LunaDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
extern TextureSamplers textureSamplers;
extern PushConstants pushConstants;
extern LunaRenderPass renderPass;
extern Pipelines pipelines;
extern Buffers buffers;
#pragma endregion variables

VkResult CreateShaderModule(const char *path, ShaderType shaderType, LunaShaderModule *shaderModule);

uint32_t TextureIndex(const char *texture);

uint32_t ImageIndex(const Image *image);

VkResult LoadSky(const ModelDefinition *skyModel);

void LoadWalls(const Map *level);

void UpdateTransformMatrix(const Camera *camera);

void UpdateViewModelMatrix(const Viewmodel *viewmodel);

void EnsureSpaceForUiElements(size_t vertexCount, size_t indexCount);

void DrawRectInternal(float ndcStartX,
					  float ndcStartY,
					  float ndcEndX,
					  float ndcEndY,
					  float startU,
					  float startV,
					  float endU,
					  float endV,
					  const Color *color,
					  uint32_t textureIndex);

void DrawQuadInternal(const mat4 vertices_posXY_uvZW, const Color *color, uint32_t textureIndex);

#endif //VULKANHELPERS_H
