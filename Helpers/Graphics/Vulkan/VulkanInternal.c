// Created by Noah on 11/23/2024.
//

#include "VulkanInternal.h"
#include <assert.h>
#include <luna/luna.h>
#include <SDL_vulkan.h>
#include "../../../Structs/GlobalState.h"
#include "../../Core/Error.h"
#include "../../Core/MathEx.h"
#include "VulkanHelpers.h"
#include "VulkanResources.h"

static SDL_Window *vulkanWindow;
static VkSurfaceKHR surface;
static VkPhysicalDeviceLimits physicalDeviceLimits;

bool CreateInstance(SDL_Window *window)
{
	vulkanWindow = window;
	uint32_t extensionCount;
	if (SDL_Vulkan_GetInstanceExtensions(vulkanWindow, &extensionCount, NULL) == SDL_FALSE)
	{
		VulkanLogError("Failed to acquire extensions required for SDL window!\n");
		return false;
	}
	const char *extensionNames[extensionCount];
	if (SDL_Vulkan_GetInstanceExtensions(vulkanWindow, &extensionCount, extensionNames) == SDL_FALSE)
	{
		VulkanLogError("Failed to acquire extensions required for SDL window!\n");
		return false;
	}
	const LunaInstanceCreationInfo instanceCreationInfo = {
		.apiVersion = VK_API_VERSION_1_2,

		.extensionCount = extensionCount,
		.extensionNames = extensionNames,

#ifdef VK_ENABLE_VALIDATION_LAYER
		.enableValidation = true,
#endif
	};
	VulkanTest(lunaCreateInstance(&instanceCreationInfo), "Failed to create instance!");

	return true;
}

bool CreateSurface()
{
	if (SDL_Vulkan_CreateSurface(vulkanWindow, lunaGetInstance(), &surface) == SDL_FALSE)
	{
		VulkanLogError("Failed to create window surface\n");
		return false;
	}

	return true;
}

bool CreateLogicalDevice()
{
	const VkPhysicalDeviceFeatures vulkan10Features = {
		.multiDrawIndirect = VK_TRUE,
		.drawIndirectFirstInstance = VK_TRUE,
	};
	VkPhysicalDeviceVulkan12Features vulkan12Features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.runtimeDescriptorArray = VK_TRUE,
		.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
		.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
	};
	const VkPhysicalDeviceFeatures2 requiredFeatures = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &vulkan12Features,
		.features = vulkan10Features,
	};
	const LunaDeviceCreationInfo2 deviceCreationInfo = {
		.extensionCount = 1,
		.extensionNames = (const char *const[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
		.requiredFeatures = requiredFeatures,
		.surface = surface,
	};
	VulkanTest(lunaAddNewDevice2(&deviceCreationInfo), "Failed to create logical device!");
	physicalDeviceLimits = lunaGetPhysicalDeviceProperties().limits;
	assert(sizeof(PushConstants) <= physicalDeviceLimits.maxPushConstantsSize);
	return true;
}

bool CreateSwapchain()
{
	if (minimized)
	{
		return true;
	}

	VkSurfaceCapabilitiesKHR capabilities;
	VulkanTest(lunaGetSurfaceCapabilities(surface, &capabilities), "Failed to get surface capabilities!");

	if (!capabilities.currentExtent.width || !capabilities.currentExtent.height)
	{
		// Window is minimized, so return to prevent creating a swap chain with dimensions of 0px by 0px
		// However, we do not want to fail or even log anything, since this is intended behavior
		minimized = true;
		return true;
	}

	swapChainExtent = capabilities.currentExtent;
	if (swapChainExtent.width == UINT32_MAX || swapChainExtent.height == UINT32_MAX)
	{
		int32_t width;
		int32_t height;
		SDL_Vulkan_GetDrawableSize(vulkanWindow, &width, &height);
		swapChainExtent.width = clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		swapChainExtent.height = clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	}

	const bool vsync = GetState()->options.vsync;
	VkPresentModeKHR presentModes[3];
	if (vsync)
	{
		presentModes[0] = VK_PRESENT_MODE_FIFO_KHR;
	} else
	{
		presentModes[0] = VK_PRESENT_MODE_MAILBOX_KHR;
		presentModes[1] = VK_PRESENT_MODE_IMMEDIATE_KHR;
		presentModes[2] = VK_PRESENT_MODE_FIFO_KHR;
	}

	const LunaSwapchainCreationInfo swapChainCreationInfo = {
		.surface = surface,
		.width = swapChainExtent.width,
		.height = swapChainExtent.height,
		.formatCount = 2,
		.formatPriorityList = (VkSurfaceFormatKHR[]){{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
													 {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}},
		.presentModeCount = vsync ? 1 : 3,
		.presentModePriorityList = presentModes,
	};

	VulkanTest(lunaCreateSwapchain(&swapChainCreationInfo), "Failed to create swap chain!");

	return true;
}

bool CreateRenderPass()
{
	// TODO: Once Luna supports it, prefer using VK_FORMAT_D32_SFLOAT
	//  Also ensure that that is the best format for all drivers, not just for NVIDIA
	lunaSetDepthImageFormat(2, (VkFormat[]){VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT});

	switch (GetState()->options.msaa)
	{
		case MSAA_2X:
			msaaSamples = VK_SAMPLE_COUNT_2_BIT;
			break;
		case MSAA_4X:
			msaaSamples = VK_SAMPLE_COUNT_4_BIT;
			break;
		case MSAA_8X:
			msaaSamples = VK_SAMPLE_COUNT_8_BIT;
			break;
		case MSAA_NONE:
		default:
			msaaSamples = VK_SAMPLE_COUNT_1_BIT;
			break;
	}
	if (!(physicalDeviceLimits.framebufferColorSampleCounts &
		  physicalDeviceLimits.framebufferDepthSampleCounts &
		  msaaSamples))
	{
		while (!(physicalDeviceLimits.framebufferColorSampleCounts &
				 physicalDeviceLimits.framebufferDepthSampleCounts &
				 msaaSamples))
		{
			msaaSamples >>= 1;
			if (msaaSamples == 0)
			{
				VulkanLogError("Found device does not support sampling the image even once.");
				return false;
			}
		}
		ShowWarning("Invalid Settings",
					"Your GPU driver does not support the selected MSAA level!\n"
					"A fallback has been set to avoid issues.");
	}

	const LunaSubpassCreationInfo subpassCreationInfo = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.useColorAttachment = true,
		.useDepthAttachment = true,
	};
	VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	};
	SDL_Rect bounds;
	SDL_GetDisplayBounds(SDL_GetWindowDisplayIndex(vulkanWindow), &bounds);
	const LunaRenderPassCreationInfo renderPassCreationInfo = {
		.samples = msaaSamples,
		.createColorAttachment = true,
		.colorAttachmentLoadMode = LUNA_ATTACHMENT_LOAD_CLEAR,
		.createDepthAttachment = true,
		.depthAttachmentLoadMode = LUNA_ATTACHMENT_LOAD_CLEAR,
		.subpassCount = 1,
		.subpasses = &subpassCreationInfo,
		.dependencyCount = 1,
		.dependencies = &dependency,
		.extent = (VkExtent3D){.width = swapChainExtent.width, .height = swapChainExtent.height, .depth = 1},
		.maxExtent = (VkExtent3D){.width = bounds.w, .height = bounds.h, .depth = 1},
	};
	VulkanTest(lunaCreateRenderPass(&renderPassCreationInfo, &renderPass), "Failed to create render pass!");
	return true;
}

bool CreateDescriptorSetLayouts()
{
	const LunaDescriptorSetLayoutBinding binding = {
		.bindingName = "Textures",
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = MAX_TEXTURES,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.bindingFlags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
	};
	const LunaDescriptorSetLayoutCreationInfo descriptorSetLayoutCreationInfo = {
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.bindingCount = 1,
		.bindings = &binding,
	};
	VulkanTest(lunaCreateDescriptorSetLayout(&descriptorSetLayoutCreationInfo, &descriptorSetLayout),
			   "Failed to create pipeline descriptor set layout!");

	return true;
}

bool CreateTextureSamplers()
{
	const LunaSamplerCreationInfo linearRepeatSamplerCreateInfo = {
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = -1.5f,
		.maxLod = VK_LOD_CLAMP_NONE,
	};
	const LunaSamplerCreationInfo nearestRepeatSamplerCreateInfo = {
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = -1.5f,
		.maxLod = VK_LOD_CLAMP_NONE,
	};
	const LunaSamplerCreationInfo linearNoRepeatSamplerCreateInfo = {
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipLodBias = -1.5f,
		.maxLod = VK_LOD_CLAMP_NONE,
	};

	const LunaSamplerCreationInfo nearestNoRepeatSamplerCreateInfo = {
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipLodBias = -1.5f,
		.maxLod = VK_LOD_CLAMP_NONE,
	};
	VulkanTest(lunaCreateSampler(&linearRepeatSamplerCreateInfo, &textureSamplers.linearRepeat),
			   "Failed to create linear repeating texture sampler!");
	VulkanTest(lunaCreateSampler(&nearestRepeatSamplerCreateInfo, &textureSamplers.nearestRepeat),
			   "Failed to create nearest repeating texture sampler!");
	VulkanTest(lunaCreateSampler(&linearNoRepeatSamplerCreateInfo, &textureSamplers.linearNoRepeat),
			   "Failed to create linear non-repeating texture sampler!");
	VulkanTest(lunaCreateSampler(&nearestNoRepeatSamplerCreateInfo, &textureSamplers.nearestNoRepeat),
			   "Failed to create nearest non-repeating texture sampler!");

	ListInit(textures, LIST_POINTER);
	memset(imageAssetIdToIndexMap, -1, sizeof(*imageAssetIdToIndexMap) * MAX_TEXTURES);

	return true;
}

bool CreateBuffers()
{
	VulkanTest(CreateUiBuffers(), "Failed to create UI buffers!");
	VulkanTest(CreateViewModelBuffers(), "Failed to create view model buffers!");
	VulkanTest(CreateWallBuffers(), "Failed to create wall buffers!");
	VulkanTest(CreateActorWallBuffers(), "Failed to create wall actor buffers!");
	VulkanTest(CreateActorModelBuffers(), "Failed to create model actor buffers!");

	return true;
}

bool CreateDescriptorSets()
{
	LunaDescriptorPool descriptorPool;
	const VkDescriptorPoolSize poolSize = {
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT,
	};
	const LunaDescriptorPoolCreationInfo descriptorPoolCreationInfo = {
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.maxSets = MAX_FRAMES_IN_FLIGHT,
		.poolSizeCount = 1,
		.poolSizes = &poolSize,
	};
	VulkanTest(lunaCreateDescriptorPool(&descriptorPoolCreationInfo, &descriptorPool),
			   "Failed to create descriptor pool!");

	LunaDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		layouts[i] = descriptorSetLayout;
	}
	const LunaDescriptorSetAllocationInfo allocationInfo = {
		.descriptorPool = descriptorPool,
		.descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
		.setLayouts = layouts,
	};
	VulkanTest(lunaAllocateDescriptorSets(&allocationInfo, descriptorSets), "Failed to allocate descriptor sets!");

	return true;
}
