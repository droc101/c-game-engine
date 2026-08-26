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
#define FRAMES_IN_FLIGHT 1

#define SizeofMember(Type, member) (sizeof(((Type *)0)->member))

#define VulkanLogError(...) LogInternal("VULKAN", 31, true, __VA_ARGS__)
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
		if (resizeCheckResult != VK_SUCCESS && resizeCheckResult != VK_SUBOPTIMAL_KHR) \
		{ \
			if (resizeCheckResult == VK_ERROR_OUT_OF_DATE_KHR) \
			{ \
				const Vector2 windowSize = ActualWindowSizeIgnoreDPI(); \
				const LunaSwapchainResizeInfo swapchainResizeInfo = { \
					.newSize.width = windowSize.x, \
					.newSize.height = windowSize.y, \
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
	mat4 view;
	Vector3 position;
	float nearPlane;
	float farPlane;
	float frustumPlanes[4];
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
	Color modColor;
	Color materialColor;
	uint32_t textureIndex;
	float _padding[7]; // I hate alignment
} ActorModelInstanceData;

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

typedef struct FrustumCullingData
{
	mat4 viewMatrix;
	float nearPlane;
	float farPlane;
	float frustumPlanes[4];


	VkDeviceAddress shadedOpaqueMapModelsCullingInfo;
	VkDeviceAddress shadedOpaqueMapModelsUnculledDrawInfo;
	VkDeviceAddress shadedOpaqueMapModelsOutputDrawInfo;

	VkDeviceAddress unshadedOpaqueMapModelsCullingInfo;
	VkDeviceAddress unshadedOpaqueMapModelsUnculledDrawInfo;
	VkDeviceAddress unshadedOpaqueMapModelsOutputDrawInfo;

	VkDeviceAddress shadedMapModelsCullingInfo;
	VkDeviceAddress shadedMapModelsUnculledDrawInfo;
	VkDeviceAddress shadedMapModelsOutputDrawInfo;

	VkDeviceAddress unshadedMapModelsCullingInfo;
	VkDeviceAddress unshadedMapModelsUnculledDrawInfo;
	VkDeviceAddress unshadedMapModelsOutputDrawInfo;

	VkDeviceAddress shadedActorWallsCullingInfo;
	VkDeviceAddress shadedActorWallsInstanceIndices;
	VkDeviceAddress shadedActorWallsDrawInfo;

	VkDeviceAddress unshadedActorWallsCullingInfo;
	VkDeviceAddress unshadedActorWallsInstanceIndices;
	VkDeviceAddress unshadedActorWallsDrawInfo;

	VkDeviceAddress shadedActorModelsCullingInfo;
	VkDeviceAddress shadedActorModelsInstanceIndices;
	VkDeviceAddress shadedActorModelsDrawInfo;

	VkDeviceAddress unshadedActorModelsCullingInfo;
	VkDeviceAddress unshadedActorModelsInstanceIndices;
	VkDeviceAddress unshadedActorModelsDrawInfo;


	float _padding[2];
} FrustumCullingData;

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

typedef struct ActorModelsBuffer
{
	/// A buffer containing per-vertex data
	LunaBuffer vertices;
	/// A buffer containing the index data to use along-side the per-vertex data
	LunaBuffer indices;
	/// A buffer containing the instance data for each instance of each model section
	LunaBuffer instanceData;
	/// A list of LunaBuffer handles containing the uint32_t indices used to get the instance data for each instance to draw
	List shadedInstanceIndices;
	/// A list of LunaBuffer handles containing the uint32_t indices used to get the instance data for each instance to draw
	List unshadedInstanceIndices;
	/// A list of LunaBuffer handles containing the structures required for the shaded materials indirect draw calls
	List shadedDrawInfo;
	/// A list of LunaBuffer handles containing the structures required for the unshaded materials indirect draw calls
	List unshadedDrawInfo;
	/// A buffer containing information about each shaded material instance that is used for culling
	LunaBuffer shadedCullingInfo;
	/// A buffer containing information about each unshaded material instance that is used for culling
	LunaBuffer unshadedCullingInfo;
} ActorModelsBuffer;

typedef struct ActorWallsBuffer
{
	/// The number of shaded actor walls in the map
	uint32_t shadedInstanceCount;
	/// The number of unshaded actor walls in the map
	uint32_t unshadedInstanceCount;
	/// A buffer of the 12 ActorWallVertex values corresponding to the two faces of the quad
	LunaBuffer vertices;
	/// A buffer containing the ActorWallInstanceData for each actor wall
	LunaBuffer instanceData;
	/// A list of LunaBuffer handles containing uint32_t instance indices emitted by culling for each shaded actor wall
	List shadedInstanceIndices;
	/// A list of LunaBuffer handles containing uint32_t instance indices emitted by culling for each unshaded actor wall
	List unshadedInstanceIndices;
	/// A list of LunaBuffer handles containing the VkDrawIndirectCommand for each shaded actor wall
	List shadedDrawInfo;
	/// A list of LunaBuffer handles containing the VkDrawIndirectCommand for each unshaded actor wall
	List unshadedDrawInfo;
	/// A buffer containing information about each shaded actor wall that is used for culling
	LunaBuffer shadedCullingInfo;
	/// A buffer containing information about each unshaded actor wall that is used for culling
	LunaBuffer unshadedCullingInfo;
} ActorWallsBuffer;

typedef struct MapModelsBuffer
{
	/// A buffer containing per-vertex data
	LunaBuffer vertices;
	/// A buffer containing the index data to use along-side the per-vertex data
	LunaBuffer indices;
	/// A buffer containing the instance data for each instance of each model section
	LunaBuffer instanceData;
	/// A buffer containing the VkDrawIndexedIndirectCommand structures required for the shaded materials draw call
	LunaBuffer unculledShadedDrawInfo;
	/// A buffer containing the VkDrawIndexedIndirectCommand structures required for the unshaded materials draw call
	LunaBuffer unculledUnshadedDrawInfo;
	/// A list of LunaBuffer handles containing the structures required for the shaded materials indirect draw calls
	List shadedDrawInfo;
	/// A list of LunaBuffer handles containing the structures required for the unshaded materials indirect draw calls
	List unshadedDrawInfo;
	/// A buffer containing information about each shaded material instance that is used for culling
	LunaBuffer shadedCullingInfo;
	/// A buffer containing information about each unshaded material instance that is used for culling
	LunaBuffer unshadedCullingInfo;
} MapModelsBuffer;

typedef struct ModelBuffer
{
	/// A buffer containing per-vertex data
	LunaBuffer vertices;
	/// A buffer containing the index data to use along-side the per-vertex data
	LunaBuffer indices;
	/// A buffer containing the instance data for each instance of each model section
	LunaBuffer instanceData;
	/// A buffer containing the structures required for the shaded materials indirect draw call
	LunaBuffer shadedDrawInfo;
	/// A buffer containing the structures required for the unshaded materials indirect draw call
	LunaBuffer unshadedDrawInfo;
} ModelBuffer;

typedef struct SkyBuffer
{
	LunaBuffer vertices;
	LunaBuffer indices;
} SkyBuffer;

typedef struct PlayerModelBuffer
{
	ModelBuffer buffers;
	ModelInstanceData *instanceData;
	ModelDefinition *modelDefinition;
} PlayerModelBuffer;

#ifdef JPH_DEBUG_RENDERER
// TODO: Clean up both this and the whole system
typedef struct DebugDrawBuffer
{
	LunaBuffer buffer;
	void *vertices;
	VkDeviceSize bytesUsed;
	VkDeviceSize allocatedSize;
} DebugDrawBuffer;
#endif

typedef struct Buffers
{
	UiBuffer ui;
	UniformBuffers uniforms;
	/// A buffer containing one FrustumCullingData structure per frustum
	LunaBuffer frustums;
	ActorModelsBuffer actorModels;
	ActorWallsBuffer actorWalls;
	MapModelsBuffer opaqueMap;
	MapModelsBuffer map;
	ModelBuffer viewmodel;
	SkyBuffer sky;
	PlayerModelBuffer player;
#ifdef JPH_DEBUG_RENDERER
	DebugDrawBuffer debugDrawLines;
	DebugDrawBuffer debugDrawTriangles;
#endif
} Buffers;

typedef struct ShadowMapPipelines
{
	LunaGraphicsPipeline opaqueMap;
	LunaGraphicsPipeline opaqueModelActors;
	LunaGraphicsPipeline opaqueWallActors;
	LunaGraphicsPipeline map;
	LunaGraphicsPipeline modelActors;
	LunaGraphicsPipeline wallActors;
} ShadowMapPipelines;

typedef struct DirectionalShadowMapPipelines
{
	LunaGraphicsPipeline mapFrontFaces;
	LunaGraphicsPipeline mapBackFaces;
	LunaGraphicsPipeline modelActors;
	LunaGraphicsPipeline wallActors;
} DirectionalShadowMapPipelines;

typedef struct Pipelines
{
	LunaComputePipeline culling;
	LunaComputePipeline clearCullingData;

	LunaGraphicsPipeline ui;
	LunaGraphicsPipeline shadedMap;
	LunaGraphicsPipeline unshadedMap;
	LunaGraphicsPipeline sky;
	LunaGraphicsPipeline shadedModel;
	LunaGraphicsPipeline unshadedModel;
	LunaGraphicsPipeline shadedActorModel;
	LunaGraphicsPipeline unshadedActorModel;
	LunaGraphicsPipeline shadedActorWall;
	LunaGraphicsPipeline unshadedActorWall;
	ShadowMapPipelines shadowMaps;
	DirectionalShadowMapPipelines directionalLightShadowMaps;
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
	LunaSampler shadowMaps;
} TextureSamplers;

typedef struct ShadowMapPushConstants
{
	uint32_t lightIndex;
	uint32_t faceIndex;
	uint32_t cascadeIndex;
} ShadowMapPushConstants;

typedef struct DescriptorSet
{
	LunaDescriptorSetLayout layout;
	LunaDescriptorSet set;
} DescriptorSet;

typedef struct DescriptorSets
{
	DescriptorSet common;
	DescriptorSet culling;
	DescriptorSet spotLightShadowMaps;
	DescriptorSet pointLightShadowMaps;
} DescriptorSets;

typedef struct CullingInfo
{
	Vector3 position;
	float radius;
	uint32_t castsShadows;
} CullingInfo;

typedef struct ModelActorCullingInfo
{
	Vector3 position;
	float radius;
	uint32_t castsShadows;
	uint32_t drawInfoIndex;
} ModelActorCullingInfo;

typedef struct DrawActorModelIndexedIndirectCommand
{
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int vertexOffset;
	uint32_t firstInstance;

	uint32_t instanceIndex;
} DrawActorModelIndexedIndirectCommand;
#pragma endregion typedefs

#pragma region variables
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
extern VkSampleCountFlagBits msaaSamples;
extern LunaRenderPass renderPass;
extern uint32_t imageAssetIdToIndexMap[MAX_TEXTURES];
extern TextureSamplers textureSamplers;
extern LockingList textures;
extern DescriptorSets descriptorSets;
extern Buffers buffers;
extern Pipelines pipelines;
extern uint32_t pendingTasks; // Bits set with PendingTasksBitFlags
extern uint32_t skyTextureIndex;
extern uint32_t shadowMapSlotsAvailable;
extern ShadowMapPushConstants shadowMapPushConstants;
extern VkRenderPass shadowMapRenderPass;
extern List shadowMaps;
extern List shadowMapFramebuffers;
extern List pointLightShadowMapImageViews;
extern List perFrustumBuffersHandles;
extern uint32_t frustumCount;
extern FrustumCullingData *frustums;

/// Simply a collection of constants that are used to prevent significant usage of magic numbers
enum PerFrustumBufferMagicConstants : uint32_t
{
	PER_FRUSTUM_BUFFER_MODEL_ACTOR_DRAW_INFO_OFFSET = 0,
	PER_FRUSTUM_BUFFER_WALL_ACTOR_DRAW_INFO_OFFSET = 2,
	PER_FRUSTUM_BUFFER_OPAQUE_MAP_DRAW_INFO_OFFSET = 4,
	PER_FRUSTUM_BUFFER_MAP_DRAW_INFO_OFFSET = 6,
	PER_FRUSTUM_BUFFER_MODEL_ACTOR_INSTANCE_INDICES_OFFSET = 8,
	PER_FRUSTUM_BUFFER_WALL_ACTOR_INSTANCE_INDICES_OFFSET = 10,
	PER_FRUSTUM_BUFFER_COUNT = 12,
};
#pragma endregion variables

bool ClearTextureCache();

bool ClearModelCache();

VkResult CreateShaderModule(const char *path, ShaderType shaderType, LunaShaderModule *shaderModule);

uint32_t TextureIndex(const char *texture);

uint32_t ImageIndex(const Image *image);

uint32_t ShadowMapResolution(LightType type);

VkResult CreateShadowMapRenderPass(const Map *map);

VkResult CreateShadowMapGraphicsPipelines();

VkResult UpdateCameraUniform(Camera *camera);

VkResult UpdateViewModelMatrix(const Viewmodel *viewmodel);

VkResult UpdateDirectionalLightCascades(const Camera *camera, const Light *light);

VkResult WriteFrustumsBuffer();

VkResult CullModels();

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
