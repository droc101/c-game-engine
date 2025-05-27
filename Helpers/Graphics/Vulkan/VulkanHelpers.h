//
// Created by Noah on 11/23/2024.
//

#ifndef VULKANHELPERS_H
#define VULKANHELPERS_H

#include <luna/lunaTypes.h>
#include "../../Core/AssetReader.h"
#include "../../Core/Logging.h"

#pragma region macros
#define MAX_FRAMES_IN_FLIGHT 1
#define MAX_UI_QUADS_INIT 8192 // TODO: find best value
#define MAX_WALLS_INIT 1024
#define MAX_WALL_ACTORS_INIT 256
#define MAX_MODEL_ACTOR_QUADS_INIT 4096

#define VulkanLogError(...) LogInternal("VULKAN", 31, true, __VA_ARGS__)
// TODO Use LogInternal
#define VulkanTestInternal(function, returnValue, ...) \
	{ \
		const VkResult result = function; \
		if (result != VK_SUCCESS) \
		{ \
			printf("\033[31m"); \
			printf(__VA_ARGS__); \
			printf("\033[0m Error code %d\n", result); \
			if (result == VK_ERROR_DEVICE_LOST) \
			{ \
				printf("See https://starflight.dev/media/VK_ERROR_DEVICE_LOST.webp for more information\n"); \
			} \
			fflush(stdout); \
			return returnValue; \
		} \
	}
#define VulkanTestReturnResult(function, ...) VulkanTestInternal(function, result, __VA_ARGS__)
#define VulkanTest(function, ...) VulkanTestInternal(function, false, __VA_ARGS__)
#pragma endregion macros

#pragma region typedefs
typedef enum VendorIDs
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
} VendorIDs;

typedef struct ActorVertex
{
	/// The x component of the vertex's position, in model space
	float x;
	/// The y component of the vertex's position, in model space
	float y;
	/// The z component of the vertex's position, in model space
	float z;

	/// The u component of the vertex's uv
	float u;
	/// The v component of the vertex's uv
	float v;

	/// The x component of the vertex's normal vector, in model space
	float nx;
	/// The y component of the vertex's normal vector, in model space
	float ny;
	/// The z component of the vertex's normal vector, in model space
	float nz;
} ActorVertex;

typedef struct ActorInstanceData
{
	/// The instance's transformation matrix.
	mat4 transform;
	/// The instance's texture index.
	uint32_t textureIndex;
	/// The instance's rotation, if it is a wall, otherwise undefined.
	float wallAngle;
} ActorInstanceData;

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

typedef struct ShadowVertex
{
	float x;
	float y;
	float z;
} ShadowVertex;

typedef struct WallVertex
{
	float x;
	float y;
	float z;

	float u;
	float v;

	uint32_t textureIndex; // TODO Per-vertex is less than ideal
	float wallAngle;
} WallVertex;

typedef struct BufferRegion
{
	LunaBuffer buffer;
	VkDeviceSize bytesUsed;
	VkDeviceSize allocatedSize;
	void *data;
} BufferRegion;

typedef struct IndexedVertexBuffer
{
	BufferRegion vertices;
	BufferRegion indices;
	uint32_t objectCount;
	bool shouldResize;
} IndexedVertexBuffer;

typedef struct RoofBuffer
{
	BufferRegion vertices;
	BufferRegion indices;
} RoofBuffer;

typedef struct WallActorsBuffer
{
	BufferRegion vertices;
	BufferRegion indices;
	BufferRegion instanceData;
	BufferRegion drawInfo;
	VkDeviceSize count;
} WallActorsBuffer;

typedef struct ModelActorsBuffer
{
	BufferRegion vertices;
	BufferRegion indices;
	BufferRegion instanceData;
	BufferRegion drawInfo;

	/// A list of the ids of all loaded actor models in the current level. This can be used in conjunction with
	/// @c ListFind to get an index that can be used to index nearly every other array in this struct.
	List loadedModelIds;
	/// An array containing the number of instances of each model index in the level.
	List modelCounts;
} ModelActorsBuffer;

typedef struct Buffers
{
	IndexedVertexBuffer ui;
	RoofBuffer roof;
	IndexedVertexBuffer walls;
	IndexedVertexBuffer shadows;
	WallActorsBuffer wallActors;
	ModelActorsBuffer modelActors;
} Buffers;

typedef struct Pipelines
{
	LunaGraphicsPipeline walls;
	LunaGraphicsPipeline actors;
	LunaGraphicsPipeline ui;
} Pipelines;

typedef struct TextureSamplers
{
	LunaSampler linearRepeat;
	LunaSampler nearestRepeat;
	LunaSampler linearNoRepeat;
	LunaSampler nearestNoRepeat;
} TextureSamplers;

typedef struct PushConstants
{
	vec2 position;
	float yaw;
	mat4 translationMatrix;

	uint32_t skyVertexCount;
	uint32_t skyTextureIndex;

	uint32_t shadowTextureIndex;

	float fogStart;
	float fogEnd;
	uint fogColor;
} PushConstants;
#pragma endregion typedefs

#pragma region variables
extern SDL_Window *vulkanWindow;
extern bool minimized;
extern bool shouldDropFrame;
extern size_t loadedActors;

extern VkSurfaceKHR surface;
extern VkPhysicalDeviceLimits physicalDeviceLimits;
extern VkExtent2D swapChainExtent;
extern LunaRenderPass renderPass;
extern LunaDescriptorSetLayout descriptorSetLayout;
extern Pipelines pipelines;
extern uint8_t currentFrame;
extern Buffers buffers;
extern LunaDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
extern List textures;
extern uint32_t imageAssetIdToIndexMap[MAX_TEXTURES];
extern TextureSamplers textureSamplers;
extern VkClearColorValue clearColor;
extern VkSampleCountFlagBits msaaSamples;
extern PushConstants pushConstants;
#pragma endregion variables

#pragma region helperFunctions
VkResult CreateShaderModule(const char *path, LunaShaderModule *shaderModule);

uint32_t TextureIndex(const char *texture);

uint32_t ImageIndex(const Image *image);

void LoadRoof(bool hasCeiling, uint32_t ceilingTextureIndex);

void LoadWalls(const Level *level);

VkResult UpdateActorInstanceDataAndShadows(const Level *level);
#pragma endregion helperFunctions

#pragma region drawingHelpers
void UpdateTranslationMatrix(const Camera *camera);

void DrawRectInternal(float ndcStartX,
					  float ndcStartY,
					  float ndcEndX,
					  float ndcEndY,
					  float startU,
					  float startV,
					  float endU,
					  float endV,
					  Color color,
					  uint32_t textureIndex);

void DrawQuadInternal(const mat4 vertices_posXY_uvZW, Color color, uint32_t textureIndex);
#pragma endregion drawingHelpers

#endif //VULKANHELPERS_H
