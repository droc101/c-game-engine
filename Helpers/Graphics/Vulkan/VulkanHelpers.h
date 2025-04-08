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

/**
 * A structure holding data about a Vulkan buffer.
 *
 * This structure is used to keep track of the buffer handle, as well as the memory allocation information associated
 * with the given buffer.
 *
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkBuffer.html
 */
typedef struct Buffer
{
	/// The actual Vulkan buffer handle
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkBuffer.html
	VkBuffer buffer;
	/// The size of the buffer allocation.
	VkDeviceSize size;
	/// Stores information about what memory contains the buffer as well as where the buffer is in the memory.
	MemoryAllocationInfo memoryAllocationInfo;
} Buffer;

/**
 * A structure holding data about a UI vertex buffer.
 *
 * This structure is used to keep track of not only the larger buffer that the vertex buffer is offset into, but also to
 * keep track of the host mapped memory and vertex count information.
 *
 * @note This is still C, so there are no actual guardrails preventing you from potentially causing a SEGFAULT by attempting to write to @c vertices[100] when @c maxQuads is only 5.
 */
typedef struct UiBuffer
{
	/// A fallback pointer that can be used if it is necessary to write more than @code maxQuads * 4@endcode vertices to the buffer.
	/// @note This pointer takes the form of @code UiVertex[fallbackMaxQuads * 4]@endcode.
	/// @note This pointer is host only, meaning that there is no Vulkan memory backing it. This means that for the GPU to be able to access it the data must first be copied to a buffer.
	UiVertex *vertices;
	/// A fallback pointer that can be used if it is necessary to write more than @code maxQuads * 6@endcode indices to the buffer.
	/// @note This pointer takes the form of @code uint32_t[fallbackMaxVertices * 6]@endcode.
	/// @note This pointer is host only, meaning that there is no Vulkan memory backing it. This means that for the GPU to be able to access it the data must first be copied to a buffer.
	uint32_t *indices;
	/// The current number of quads that are stored in the buffer.
	uint32_t quadCount;
	/// The maximum number of quads that can be stored in the buffer with the currently allocated memory.
	uint32_t maxQuads;

	LunaBuffer vertexBuffer;
	LunaBuffer indexBuffer;

	bool shouldResize;
} UiBuffer;

/**
 * A structure holding data about a wall vertex buffer.
 *
 * This structure is used to keep track of the larger buffer that the buffer is a part of, offset information,
 * and information about the number of walls.
 */
typedef struct WallVertexBuffer
{
	/// The number of walls that are currently stored in the buffer, plus one for the floor and one for the ceiling.
	uint32_t wallCount;
	uint32_t shadowCount;
	/// The number of indices required to draw the sky.
	uint32_t skyIndexCount;
	/// The maximum number of walls that can currently be stored in the buffer.
	/// @note In order to determine how many walls can be used you must subtract either one or two from this number,
	///  due to the inclusion of the floor as a wall. If the level uses a sky, then only the floor will be part of this
	///  number, and so you will only need to subtract one, but if the level uses a ceiling you need to subtract two.
	///  VK_LoadLevelWalls handles this by adding to the wall count accordingly, so that comparing the wall count and
	///  the max wall count requires no additional arithmetic.
	uint32_t maxWallCount;

	/// The larger buffer within which the wall vertex buffer reside.
	Buffer *bufferInfo;
	/// The offset of the wall vertex buffer into the larger buffer allocation.
	VkDeviceSize vertexOffset;
	/// The offset of the index buffer for the wall vertex buffer into the larger buffer allocation.
	VkDeviceSize indexOffset;
	VkDeviceSize shadowOffset;
	VkDeviceSize shadowSize;

	Buffer *stagingBufferInfo;
	VkDeviceSize shadowStagingOffset;
	VkDeviceSize shadowStagingSize;
	ShadowVertex *shadowVertexStaging;
	ShadowVertex *shadowIndexStaging;
} WallVertexBuffer;

/// A struct that contains all the data needed to keep track of (but not to draw) all the actors in the current level
/// that use models. This struct should be considered a fragment, and is intended for use as part of the @c ActorBuffer
/// struct. As such, it is lacking the information required to instance and draw the actors.
///
/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
typedef struct ModelActorBuffer
{
	/// The total number of vertices across all models.
	uint32_t vertexCount;
	/// The total number of indices across all models.
	uint32_t indexCount;
	/// A list of the ids of all loaded actor models in the current level. This can be used in conjunction with
	/// @c ListFind to get an index that can be used to index nearly every other array in this struct.
	List loadedModelIds;
	/// An array containing the number of instances of each model index in the level.
	List modelCounts;

	/// The that the vertices take up within the device-local buffer.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize vertexSize;
	/// The that the indices take up within the device-local buffer.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize indexSize;
} ModelActorBuffer;

/// A struct that contains all the data needed to keep track of (but not to draw) all the actors in the current level
/// that exclusively use a wall. This struct should be considered a fragment, and is intended for use as part of the
/// @c ActorBuffer struct. As such, it is lacking the information required to instance and draw the actors.
///
/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
typedef struct WallActorBuffer
{
	/// The number of actors in the level that exclusively use a wall.
	uint32_t count;

	/// The offset into the device-local buffer at which the vertices are stored.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize vertexOffset;
	/// The offset into the device-local buffer at which the indices are stored.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize indexOffset;
	/// The that the vertices take up within the device-local buffer.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize vertexSize;
	/// The that the indices take up within the device-local buffer.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize indexSize;

	/// The offset into the shared memory buffer at which the vertices are stored.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize vertexStagingOffset;
	/// The offset into the shared memory buffer at which the indices are stored.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize indexStagingOffset;
	/// The that the vertices take up within the shared memory buffer.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize vertexStagingSize;
	/// The that the indices take up within the shared memory buffer.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize indexStagingSize;
	/// A pointer to the mapped memory of the shared memory buffer, offset to point to the region used for holding
	/// the staging copy of the vertices.
	ActorVertex *vertexStaging;
	/// A pointer to the mapped memory of the shared memory buffer, offset to point to the region used for holding
	/// the staging copy of the indices.
	uint32_t *indexStaging;
} WallActorBuffer;

/// A struct that contains all the data needed to keep track of and draw all the actors in the current level. This
/// struct contains information about both the actors that use models, and the actors that exclusively use a wall. It
/// does this by containing both a @c ModelActorBuffer and a @c WallActorBuffer struct, in addition to information that
/// pertains to the buffer as a whole. This means that any members of this struct that do not belong to either the
/// @c models or the @c walls member should be considered to apply to the buffer as a whole, and not just to one type of
/// actor.
///
/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDrawIndexedIndirectCommand.html
typedef struct ActorBuffer
{
	/// A pointer to a struct containing information about the device-local buffer that the actor data is stored in.
	Buffer *bufferInfo;
	/// A struct containing information about the portion of the buffer that stores the actors that use models.
	ModelActorBuffer models;
	/// A struct containing information about the portion of the buffer that stores the actors that only use a wall.
	WallActorBuffer walls;

	/// The offset into the device-local buffer at which the instance data is stored. This is the offset for both
	/// model and wall actors.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize instanceDataOffset;
	/// The offset into the device-local buffer at which the drawing information is stored. This is the offset for both
	/// model and wall actors.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize drawInfoOffset;
	/// The offset into the device-local buffer at which the vertices are stored. This is the base offset for the
	/// actors, and the model actors use this same offset, but the wall actors have a separate offset which allows them
	/// to make changes without modifying the walls.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize vertexOffset;
	/// The offset into the device-local buffer at which the indices are stored. This is the base offset for the actors,
	/// and the model actors use this same offset, but the wall actors have a separate offset which allows them to
	/// make changes without modifying the walls.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize indexOffset;
	/// The size that the instance data takes up within the device-local buffer. This is the combined size of both the
	/// wall actors and the model actors.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize instanceDataSize;
	/// The size that the drawing information takes up within the device-local buffer. This is the combined size of both
	/// the wall actors and the model actors.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize drawInfoSize;

	/// A pointer to a struct containing information about the shared memory buffer that the actor data is staged in.
	Buffer *stagingBufferInfo;
	/// The offset into the shared memory buffer at which the instance data is stored. This is the offset for both
	/// model and wall actors.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize instanceDataStagingOffset;
	/// The offset into the shared memory buffer at which the drawing information is stored. This is the offset for both
	/// model and wall actors.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize drawInfoStagingOffset;
	/// The size that the instance data takes up within the shared memory buffer. This is the combined size of both the
	/// wall actors and the model actors.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize instanceDataStagingSize;
	/// The size that the drawing information takes up within the shared memory buffer. This is the combined size of
	/// both the wall actors and the model actors.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
	VkDeviceSize drawInfoStagingSize;
	/// A pointer to the mapped memory of the shared memory buffer, offset to point to the region used for holding
	/// the staging copy of the instance data.
	ActorInstanceData *instanceDataStaging;
	/// A pointer to the mapped memory of the shared memory buffer, offset to point to the region used for holding
	/// the staging copy of the drawing information.
	/// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDrawIndexedIndirectCommand.html
	VkDrawIndexedIndirectCommand *drawInfoStaging;
} ActorBuffer;

typedef struct Buffers
{
	Buffer local;
	Buffer shared;
	Buffer staging;

	UiBuffer ui;
	WallVertexBuffer walls;
	ActorBuffer actors;
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
bool LoadActors(const Level *level);

bool CreateImageView(VkImageView *imageView,
					 VkImage image,
					 VkFormat format,
					 VkImageAspectFlagBits aspectMask,
					 uint8_t mipmapLevels,
					 const char *errorMessage);

VkResult CreateShaderModule(const char *path, VkShaderModule *shaderModule);

bool CreateImage(VkImage *image,
				 VkDeviceMemory *imageMemory,
				 VkFormat format,
				 VkExtent3D extent,
				 uint8_t mipmapLevels,
				 VkSampleCountFlags samples,
				 VkImageUsageFlags usageFlags,
				 const char *imageType);

bool BeginCommandBuffer(const VkCommandBuffer *commandBuffer, VkCommandPool commandPool);

bool EndCommandBuffer(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue queue);

bool CreateBuffer(Buffer *buffer, bool newAllocation);

bool CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy *regions);

uint32_t TextureIndex(const char *texture);

uint32_t ImageIndex(const Image *image);

void CleanupSwapChain();

void CleanupColorImage();

void CleanupDepthImage();

void CleanupPipeline();

void CleanupSyncObjects();

bool RecreateSwapChain();

bool DestroyBuffer(Buffer *buffer);

void LoadWalls(const Level *level,
			   const Model *skyModel,
			   WallVertex *vertices,
			   uint32_t *indices,
			   uint32_t skyVertexCount);

void LoadActorModels(const Level *level, ActorVertex *vertices, uint32_t *indices);

void LoadActorWalls(const Level *level, ActorVertex *vertices, uint32_t *indices);

void LoadActorInstanceData(const Level *level,
						   ActorInstanceData *instanceData,
						   ShadowVertex *shadowVertices,
						   uint32_t *shadowIndices);

void LoadActorDrawInfo(const Level *level, VkDrawIndexedIndirectCommand *drawInfo);

VkResult CopyBuffers(const Level *level);
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
