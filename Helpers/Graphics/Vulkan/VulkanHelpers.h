//
// Created by Noah on 11/23/2024.
//

#ifndef VULKANHELPERS_H
#define VULKANHELPERS_H

#include <cglm/cglm.h>
#include <luna/lunaTypes.h>
#include <vulkan/vulkan.h>
#include "../../Core/AssetReader.h"
#include "../../Core/DataReader.h"

#pragma region macros
#define VULKAN_VERSION VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)
#define MAX_FRAMES_IN_FLIGHT 1
#define MAX_UI_QUADS_INIT 8192 // TODO: find best value
#define MAX_WALLS_INIT 1024
#define MAX_WALL_ACTORS_INIT 256
#define MAX_MODEL_ACTOR_QUADS_INIT 4096
/// This is an expected estimate for the largest that a texture will be. It is used to create an overallocation of
/// texture memory with the formula @code MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4 * textureCount@endcode
#define MAX_TEXTURE_SIZE 384

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
/**
 * Bit flags representing unique families that are stored in @c QueueFamilyIndices
 */
typedef enum QueueFamily
{
	QUEUE_FAMILY_GRAPHICS = 1,
	QUEUE_FAMILY_PRESENTATION = 2,
	QUEUE_FAMILY_TRANSFER = 4,
} QueueFamily;

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

typedef enum RoofType
{
	ROOF_TYPE_CEILING = 1,
	ROOF_TYPE_SKY = 2,
} RoofType;

/**
 * A struct to hold the indicies of the queue families for graphics, presentation, and transfer.
 * This is used to find and store the indices, which allows for picking between unique and non-unique indices.
 */
typedef struct QueueFamilyIndices
{
	/// The index of the family on the GPU that will be used for graphics processing
	uint32_t graphicsFamily;
	/// The index of the family on the GPU that will be used for presentation
	/// @note If the graphics family supports presentation, @c QueueFamilyIndices::presentFamily will contain the same value as @c QueueFamilyIndices::graphicsFamily
	/// @note Similarly, if the graphics family does not support presentation then @c QueueFamilyIndices::presentFamily will contain the same value as @c QueueFamilyIndices::uniquePresentFamily
	uint32_t presentFamily;
	/// If the graphics family does not support presentation this will contain the same value as @c QueueFamilyIndices::presentFamily
	uint32_t uniquePresentFamily;
	/// The index of the family on the GPU that will be used for transfer operations
	uint32_t transferFamily;
	/// A bitmask of @c QueueFamily values representing which queue families are unique.
	uint8_t families;
	/// The total count of unique families
	uint8_t familyCount;
} QueueFamilyIndices;

/**
 * A struct used to hold information about the size and type of the memory, as well as a host pointer mapped to the
 * Vulkan memory allocation.
 *
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceMemory.html
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkMemoryPropertyFlags.html
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkMemoryPropertyFlagBits.html
 */
typedef struct MemoryInfo
{
	/// The size of the block of memory.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize size;
	/// A pointer to the host memory block mapped to this block of memory.
	void *mappedMemory;
	/// The actual Vulkan memory handle.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceMemory.html
	VkDeviceMemory memory;
	/// A bitmask of VkMemoryPropertyFlagBits that describes the memory type.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkMemoryPropertyFlags.html
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkMemoryPropertyFlagBits.html
	VkMemoryPropertyFlags type;
} MemoryInfo;

/**
 * A struct used to hold information about the allocation of a certain resource out of a larger block of memory.
 *
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkMemoryRequirements.html
 */
typedef struct MemoryAllocationInfo
{
	/// The offset at which the object resides within the larger block of memory.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize offset;
	/// A pointer to a MemoryInfo struct containing more information about the larger block of memory.
	MemoryInfo *memoryInfo;
	/// Information about the allocation requirements such as size, alignment, and memory type.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkMemoryRequirements.html
	VkMemoryRequirements memoryRequirements;
	/// A bitmask of usage flags that describes what the buffer is allowed to be used for.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkBufferUsageFlags.html
	VkBufferUsageFlags usageFlags; // TODO: Move me when rewriting buffer allocation
} MemoryAllocationInfo;

typedef struct MemoryPools
{
	MemoryInfo localMemory;
	MemoryInfo sharedMemory;
	MemoryInfo stagingMemory;
} MemoryPools;

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

typedef struct PhysicalDevice
{
	/// The physical device is the hardware available to the host that has an implementation of Vulkan.
	/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-physical-device-enumeration
	/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html
	VkPhysicalDevice device;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceMemoryProperties memoryProperties;
} PhysicalDevice;

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
extern SDL_Window *vk_window;
extern bool minimized;
extern size_t loadedActors;

/// When the instance is created the Vulkan library gets initialized, allowing the game to provide the library with any
/// information about itself. Any state information that the library provides will then be stored in the instance.
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstance.html
extern VkInstance instance;
/// The interface between Vulkan and SDL, allowing Vulkan to actually interact with the window.
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSurfaceKHR.html
extern VkSurfaceKHR surface;
/// @todo Document this along with the struct
extern PhysicalDevice physicalDevice;
/// @todo Document this
extern QueueFamilyIndices queueFamilyIndices;
/// The logical device is a connection to a physical device, and is used for interfacing with Vulkan.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-devices
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDevice.html
extern VkDevice device;
/// The graphics queue is the queue used for executing graphics command buffers on the device.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-queues
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueue.html
extern VkQueue graphicsQueue;
/// The present queue is the queue used for executing present command buffers on the device.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-queues
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueue.html
extern VkQueue presentQueue;
/// The transfer queue is the queue used for executing transfer command buffers on the device.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-queues
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueue.html
extern VkQueue transferQueue;
/// Allows Vulkan to give a surface the rendered image.
extern VkSwapchainKHR swapChain;
extern VkImage *swapChainImages;
extern uint32_t swapChainCount;
extern VkFormat swapChainImageFormat;
extern VkExtent2D swapChainExtent;
extern VkImageView *swapChainImageViews;
extern LunaRenderPass renderPass;
extern LunaDescriptorSetLayout descriptorSetLayout;
extern VkPipelineCache pipelineCache;
extern Pipelines pipelines;
extern VkFramebuffer *swapChainFramebuffers;
extern VkCommandPool graphicsCommandPool;
extern VkCommandPool transferCommandPool;
extern VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
extern VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
extern VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
extern VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
extern bool framebufferResized;
extern uint8_t currentFrame;
extern uint32_t swapchainImageIndex;
extern MemoryPools memoryPools;
extern Buffers buffers;
extern LunaDescriptorPool descriptorPool;
extern LunaDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
extern List textures;
extern uint32_t imageAssetIdToIndexMap[MAX_TEXTURES];
extern TextureSamplers textureSamplers;
extern VkFormat depthImageFormat;
extern VkImage depthImage;
extern VkDeviceMemory depthImageMemory;
extern VkImageView depthImageView;
extern VkImage colorImage;
extern VkDeviceMemory colorImageMemory;
extern VkImageView colorImageView;
extern VkClearColorValue clearColor;
extern VkSampleCountFlagBits msaaSamples;
extern PushConstants pushConstants;
extern VkCommandBuffer transferCommandBuffer;
extern VkFence transferBufferFence;
#pragma endregion variables

#pragma region helperFunctions
VkResult InitActors(const Level *level);

VkResult CreateShaderModule(const char *path, VkShaderModule *shaderModule);

uint32_t TextureIndex(const char *texture);

uint32_t ImageIndex(const Image *image);

void LoadRoof(bool hasCeiling, uint32_t ceilingTextureIndex);

void LoadWalls(const Level *level);

void LoadModelActors(const Level *level);

VkResult LoadWallActors(const Level *level);

VkResult UpdateActorInstanceDataAndShadows(const Level *level);

void LoadActorDrawInfo(const Level *level);

// VkResult CopyBuffers(const Level *level);
#pragma endregion helperFunctions

#pragma region drawingHelpers
void UpdateTranslationMatrix(const Camera *camera);

VkResult BeginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex);

VkResult EndRenderPass(VkCommandBuffer commandBuffer);

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
