//
// Created by Noah on 11/23/2024.
//

#ifndef VULKANHELPERS_H
#define VULKANHELPERS_H

#include <cglm/types.h>
#include <engine/assets/ShaderLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/structs/Viewmodel.h>
#include <engine/subsystem/Logging.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <luna/lunaTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#pragma region macros
#ifdef JPH_DEBUG_RENDERER
#define MAX_DEBUG_DRAW_VERTICES_INIT 1024
#endif

#define FRAMES_IN_FLIGHT 1

#define SizeofMember(Type, member) (sizeof(((Type *)0)->member))

#define VulkanLogError(...) LogInternal("VULKAN", 31, true, __VA_ARGS__)
// TODO Use LogInternal
#define VulkanTestInternal(function, returnValue, ...) \
	{ \
		const VkResult result = function; \
		if (result != VK_SUCCESS) \
		{ \
			LogInternal("VULKAN", 31, false, __VA_ARGS__); \
			LogInternal(NULL, 31, true, "Error code: %d\n", result); \
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
				const Vector2 windowSize = ActualWindowSizeIgnoreDPI(); \
				swapChainExtent.width = windowSize.x; \
				swapChainExtent.height = windowSize.y; \
				const LunaSwapchainResizeInfo swapchainResizeInfo = { \
					.newSize = swapChainExtent, \
					.renderPassCount = 1, \
					.renderPasses = &renderPass, \
					.queueFamilyIndexCount = 1, \
					.queueFamilyIndices = &queueFamilyIndex, \
				}; \
				VulkanTest(lunaResizeSwapchain(device, &swapchainResizeInfo), "Failed to resize swapchain!"); \
				return false; \
			} \
			VulkanTest(resizeCheckResult, __VA_ARGS__); \
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

enum PendingTasksBitFlags
{
	PENDING_TASK_UI_BUFFERS_RESIZE_BIT = 1 << 0,
};

typedef struct CameraUniform
{
	mat4 transform;
	Vector3 position;
} CameraUniform;

typedef struct GlobalLightingUniform
{
	Color color;
	float exposure;
} GlobalLightingUniform;

typedef struct FogUniform
{
	Color color;
	float start;
	float end;
} FogUniform;

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

typedef struct SkyVertex
{
	/// The position of the vertex, in model space
	Vector3 position;
	/// The texture coordinate of the vertex
	Vector2 uv;
} SkyVertex;

typedef struct ActorWallVertex
{
	Vector2 position;
	Vector2 uv;
} ActorWallVertex;

typedef struct DebugDrawVertex
{
	Vector3 position;
	Color color;
} DebugDrawVertex;

typedef struct ModelInstanceData
{
	mat4 transformMatrix;
	Color materialColor;
	uint32_t textureIndex;
} ModelInstanceData;

typedef struct ActorModelInstanceData
{
	mat4 transformMatrix;
	vec4 modColor;
	vec4 materialColor;
	uint32_t textureIndex;
} __attribute__((aligned(32))) ActorModelInstanceData;

typedef struct ActorWallInstanceData
{
	Vector3 position;
	Vector2 scale;
	Vector2 axis;
	Vector2 centerOffset;
	JPH_Quat rotationQuat;
	uint32_t textureIndex;
	Vector2 uvScale;
	Vector2 uvOffset;
	Color modColor;
} ActorWallInstanceData;

typedef struct UiBuffer
{
	LunaBuffer vertexBuffer;
	LunaBuffer indexBuffer;
	uint32_t allocatedQuads;
	uint32_t freeQuads;
	UiVertex *vertexData;
	uint32_t *indexData;
} UiBuffer;

typedef struct UniformBuffers
{
	LunaBuffer camera;
	LunaBuffer lighting;
	LunaBuffer fog;
	LunaBuffer lights;
} UniformBuffers;

/**
 * Contains the required buffers for a model that can have multiple materials
 *
 * Models with multiple materials are always drawn using lunaDrawIndexedIndirect regardless of if they are instanced.
 * This is done due to the fact that each different material becomes a driver-dispatched draw call, and would have to be
 * individually dispatched draw calls if it were not using indirect draw.
 *
 * Vertex and index data does not need to be duplicated for each frame in flight due to the fact that it is in local
 * space and only the instance data should be changing on a frame-to-frame basis.
 */
typedef struct ModelBuffer
{
	/// A buffer containing per-vertex data
	LunaBuffer vertices;
	/// A buffer containing the index data to use along-side the per-vertex data
	LunaBuffer indices;
	/// A buffer containing the instance data for each instance of each model section
	LunaBuffer instanceData; //[FRAMES_IN_FLIGHT];
	/// A buffer containing the VkDrawIndexedIndirectCommand structures required for the shaded materials draw call
	LunaBuffer shadedDrawInfo; //[FRAMES_IN_FLIGHT];
	/// A buffer containing the VkDrawIndexedIndirectCommand structures required for the unshaded materials draw call
	LunaBuffer unshadedDrawInfo; //[FRAMES_IN_FLIGHT];
} ModelBuffer;

typedef struct SkyBuffer
{
	LunaBuffer vertices;
	LunaBuffer indices;
} SkyBuffer;

typedef struct ActorWallBuffer
{
	/// A buffer of the 12 ActorWallVertex values corresponding to the two faces of the quad
	LunaBuffer vertices;
	/// A buffer containing the ActorWallInstanceData for each shaded actor wall
	LunaBuffer shadedInstanceData;
	/// The number of shaded actor walls in the map
	uint32_t shadedInstanceCount;
	/// A buffer containing the ActorWallInstanceData for each unshaded actor wall
	LunaBuffer unshadedInstanceData;
	/// The number of unshaded actor walls in the map
	uint32_t unshadedInstanceCount;
} ActorWallBuffer;

#ifdef JPH_DEBUG_RENDERER
// TODO: Clean up both this and the whole system
typedef struct DebugDrawBuffer
{
	struct
	{
		LunaBuffer buffer;
		VkDeviceSize bytesUsed;
		VkDeviceSize allocatedSize;
		void *data;
	} vertices;
	uint32_t vertexCount;
	bool shouldResize;
} DebugDrawBuffer;
#endif

typedef struct Buffers
{
	UiBuffer ui; //[FRAMES_IN_FLIGHT];
	UniformBuffers uniforms; //[FRAMES_IN_FLIGHT];
	ModelBuffer viewmodel;
	ModelBuffer actorModels;
	ModelBuffer map;
	SkyBuffer sky;
	ActorWallBuffer actorWalls;
#ifdef JPH_DEBUG_RENDERER
	DebugDrawBuffer debugDrawLines;
	DebugDrawBuffer debugDrawTriangles;
#endif
} Buffers;

typedef struct Pipelines
{
	LunaGraphicsPipeline ui;
	LunaGraphicsPipeline shadedMap;
	LunaGraphicsPipeline unshadedMap;
	LunaGraphicsPipeline sky;
	LunaGraphicsPipeline shadedViewmodel;
	LunaGraphicsPipeline unshadedViewmodel;
	LunaGraphicsPipeline shadedActorModel;
	LunaGraphicsPipeline unshadedActorModel;
	LunaGraphicsPipeline shadedActorWall;
	LunaGraphicsPipeline unshadedActorWall;
#ifdef JPH_DEBUG_RENDERER
	LunaGraphicsPipeline debugDrawLines;
	LunaGraphicsPipeline debugDrawTriangles;
#endif
} Pipelines;

typedef struct TextureSamplers
{
	LunaSampler linearRepeatAnisotropy;
	LunaSampler linearNoRepeatAnisotropy;
	LunaSampler linearRepeatNoAnisotropy;
	LunaSampler nearestRepeatNoAnisotropy;
	LunaSampler linearNoRepeatNoAnisotropy;
	LunaSampler nearestNoRepeatNoAnisotropy;
} TextureSamplers;

typedef struct DescriptorSetLayouts
{
	LunaDescriptorSetLayout transform;
	LunaDescriptorSetLayout all;
	LunaDescriptorSetLayout globalLighting;
	LunaDescriptorSetLayout fog;
} DescriptorSetLayouts;
#pragma endregion typedefs

#pragma region variables
// TODO: Make sure these are all needed and are all as they should be

extern bool minimized;
extern LunaDevice device;
extern VkPhysicalDeviceProperties physicalDeviceProperties;
extern uint32_t queueFamilyIndex;
extern VkQueue queue;
extern LunaCommandPool commandPool;
extern LunaCommandBuffer commandBuffer;
extern LunaCommandBuffer secondaryCommandBuffer;
extern LunaSemaphore semaphore;
extern VkSurfaceKHR surface;
extern VkExtent2D swapChainExtent;
extern VkSampleCountFlagBits msaaSamples;
extern LunaRenderPass renderPass;
extern uint32_t imageAssetIdToIndexMap[MAX_TEXTURES];
extern TextureSamplers textureSamplers;
extern LockingList textures;
extern LunaDescriptorSetLayout descriptorSetLayout;
extern LunaDescriptorSet descriptorSet;
extern Buffers buffers;
extern Pipelines pipelines;
extern uint32_t pendingTasks; // Bits set with PendingTasksBitFlags
extern uint32_t skyTextureIndex;
#pragma endregion variables

bool ClearTextureCache();

bool ClearModelCache();

VkResult CreateShaderModule(const char *path, ShaderType shaderType, LunaShaderModule *shaderModule);

uint32_t TextureIndex(const char *texture);

uint32_t ImageIndex(const Image *image);

VkResult UpdateCameraUniform(const Camera *camera);

VkResult UpdateViewModelMatrix(const Viewmodel *viewmodel);

void EnsureSpaceForUiElements(size_t quadCount);

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
