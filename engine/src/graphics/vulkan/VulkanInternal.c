// Created by Noah on 11/23/2024.
//

#include <assert.h>
#include <engine/assets/TextureLoader.h>
#include <engine/graphics/vulkan/Vulkan.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanInternal.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Options.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <luna/luna.h>
#include <luna/lunaCommandBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaImage.h>
#include <luna/lunaInstance.h>
#include <luna/lunaSynchronization.h>
#include <luna/lunaTypes.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

static SDL_Window *vulkanWindow;

bool CreateInstance()
{
	uint32_t extensionCount = 0;
	const char *const *extensionNames = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
	if (!extensionNames)
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

bool CreateSurface(SDL_Window *window)
{
	vulkanWindow = window;

	if (!SDL_Vulkan_CreateSurface(vulkanWindow, lunaGetInstance(), NULL, &surface))
	{
		VulkanLogError("Failed to create window surface\n");
		return false;
	}

	return true;
}

bool CreateLogicalDevice()
{
	VkPhysicalDeviceType preferred = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	switch (GetState()->options.preferredGpuType)
	{
		case GPU_TYPE_INTEGRATED:
			preferred = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
			break;
		case GPU_TYPE_DEDICATED:
			preferred = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			break;
		case GPU_TYPE_SOFTWARE:
			preferred = VK_PHYSICAL_DEVICE_TYPE_CPU;
			break;
	}
	const LunaPhysicalDevicePreferenceDefinition devicePreferenceDefinition = {
		.preferredDeviceType = preferred,
	};
	const VkPhysicalDeviceFeatures vulkan10Features = {
		.samplerAnisotropy = VK_TRUE,
		.multiDrawIndirect = VK_TRUE,
		.drawIndirectFirstInstance = VK_TRUE,
	};
	VkPhysicalDeviceVulkan12Features vulkan12Features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.scalarBlockLayout = VK_TRUE,
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
		.physicalDevicePreferenceDefinition = &devicePreferenceDefinition,
	};
	VulkanTest(lunaCreateDevice2(&deviceCreationInfo, &device), "Failed to create logical device!");
	lunaGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
	// TODO: Check that no limits are being exceeded
	return true;
}

bool CreateCommandBuffers()
{
	const LunaQueueFamilyProperties requiredProperties = {
		.queueFamilyProperties.queueFlags = VK_QUEUE_GRAPHICS_BIT,
		.presentationSupport = true,
	};
	queueFamilyIndex = lunaGetDeviceQueueFamilyIndex(device, &requiredProperties);
	queue = lunaGetDeviceQueue(device, queueFamilyIndex, 0);

	const LunaCommandPoolCreationInfo commandPoolCreationInfo = {
		.queueFamilyIndex = queueFamilyIndex,
	};
	VulkanTest(lunaCreateCommandPool(device, &commandPoolCreationInfo, &commandPool), "Failed to create command pool!");
	VulkanTest(lunaAllocateCommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, &commandBuffer),
			   "Failed to allocate command buffer!");
	VulkanTest(lunaAllocateCommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, &secondaryCommandBuffer),
			   "Failed to allocate secondary command buffer!");

	const LunaSemaphoreCreationInfo semaphoreCreationInfo = {};
	VulkanTest(lunaCreateSemaphore(device, &semaphoreCreationInfo, &semaphore), "Failed to create semaphore!");

	return true;
}

// TODO: In-depth review of this function, or rewrite from the ground up
bool CreateSwapchain()
{
	if (minimized)
	{
		return true;
	}

	VkSurfaceCapabilitiesKHR capabilities;
	VulkanTest(lunaGetSurfaceCapabilities(device, surface, &capabilities), "Failed to get surface capabilities!");

	if (!capabilities.currentExtent.width || !capabilities.currentExtent.height)
	{
		// Window is minimized, so return to prevent creating a swap chain with dimensions of 0px by 0px
		// However, we do not want to fail or even log anything, since this is intended behavior
		minimized = true;
		return true;
	}

	swapchainExtent = capabilities.currentExtent;
	if (swapchainExtent.width == UINT32_MAX || swapchainExtent.height == UINT32_MAX)
	{
		int32_t width = 0;
		int32_t height = 0;
		if (!SDL_GetWindowSizeInPixels(vulkanWindow, &width, &height))
		{
			LogError("Failed to get window size with error: %s", SDL_GetError());
			return false;
		}
		swapchainExtent.width = clamp((uint32_t)width,
									  capabilities.minImageExtent.width,
									  capabilities.maxImageExtent.width);
		swapchainExtent.height = clamp((uint32_t)height,
									   capabilities.minImageExtent.height,
									   capabilities.maxImageExtent.height);
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
		.width = swapchainExtent.width,
		.height = swapchainExtent.height,
		.formatCount = 2,
		.formatPriorityList = (VkSurfaceFormatKHR[]){{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
													 {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}},
		.presentModeCount = vsync ? 1 : 3,
		.presentModePriorityList = presentModes,
		.clipped = true,

		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};

	VulkanTest(lunaCreateSwapchain(device, &swapChainCreationInfo), "Failed to create swap chain!");

	return true;
}

bool CreateRenderPass()
{
	// TODO: Once Luna supports it, prefer using VK_FORMAT_D32_SFLOAT
	//  Also ensure that that is the best format for all drivers, not just for NVIDIA
	VulkanTest(lunaSetDepthImageFormat(device,
									   2,
									   (VkFormat[]){VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT}),
			   "Failed to set depth image format!");

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
	if (!(physicalDeviceProperties.limits.framebufferColorSampleCounts &
		  physicalDeviceProperties.limits.framebufferDepthSampleCounts &
		  msaaSamples))
	{
		while (!(physicalDeviceProperties.limits.framebufferColorSampleCounts &
				 physicalDeviceProperties.limits.framebufferDepthSampleCounts &
				 msaaSamples))
		{
			msaaSamples >>= 1;
			if (msaaSamples == 0)
			{
				VulkanLogError("Found device does not support sampling the image even once. "
							   "This indicates an issue with the graphics driver.");
				return false;
			}
		}
		// TODO: This doesn't update the options and it doesn't tell the user what to change their MSAA level to
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
	if (!SDL_GetDisplayBounds(SDL_GetDisplayForWindow(vulkanWindow), &bounds))
	{
		LogError("Failed to get display bounds with error: %s", SDL_GetError());
		return false;
	}
	const LunaRenderPassCreationInfo renderPassCreationInfo = {
		.samples = msaaSamples,
		.createColorAttachment = true,
		.colorAttachmentLoadMode = LUNA_ATTACHMENT_LOAD_MODE_CLEAR,
		.createDepthAttachment = true,
		.depthAttachmentLoadMode = LUNA_ATTACHMENT_LOAD_MODE_CLEAR,
		.subpassCount = 1,
		.subpasses = &subpassCreationInfo,
		.dependencyCount = 1,
		.dependencies = &dependency,
		.extent = (VkExtent3D){.width = swapchainExtent.width, .height = swapchainExtent.height, .depth = 1},
		.maxExtent = (VkExtent3D){.width = bounds.w, .height = bounds.h, .depth = 1},

		.queueFamilyIndexCount = 1,
		.queueFamilyIndices = &queueFamilyIndex,
	};
	VulkanTest(lunaCreateRenderPass(device, &renderPassCreationInfo, &renderPass), "Failed to create render pass!");
	return true;
}

// TODO: Look into using alternative methods to remove the dependency on non-uniform indexing
bool CreateDescriptorSetLayouts()
{
	const uint32_t freeResourceCount = physicalDeviceProperties.limits.maxPerStageResources - 5;
	const uint32_t sampledImageCount = min(physicalDeviceProperties.limits.maxDescriptorSetSampledImages,
										   freeResourceCount);
	const uint32_t textureCount = min(MAX_TEXTURES, sampledImageCount);
	// TODO: This assert can actually be hit. We should account for the device limits when loading textures
	assert(MAX_TEXTURES < sampledImageCount);
	const LunaDescriptorSetLayoutBinding bindings[] = {
		{
			.bindingName = "Lightmap",
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		},
		{
			.bindingName = "Textures",
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = textureCount,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.bindingFlags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
		},
		{
			.bindingName = "Camera",
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		},
		{
			.bindingName = "Global Lighting",
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		},
		{
			.bindingName = "Fog",
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		},
		{
			.bindingName = "Lights",
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		},
		{
			// Directional light shadow maps
			.bindingName = "Shadow Maps",
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			// TODO: This is count is unchecked and not used when checking how many other shadow maps to make
			.descriptorCount = 4,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		},
	};
	const LunaDescriptorSetLayoutCreationInfo descriptorSetLayoutCreationInfo = {
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.bindingCount = sizeof(bindings) / sizeof(*bindings),
		.bindings = bindings,
	};
	VulkanTest(lunaCreateDescriptorSetLayout(device, &descriptorSetLayoutCreationInfo, &descriptorSetLayout),
			   "Failed to create descriptor set layout!");


	const uint32_t shadowMapDescriptorSlots = freeResourceCount < textureCount ? 0 : freeResourceCount - textureCount;
	shadowMapSlotsAvailable = min(min(physicalDeviceProperties.limits.maxDescriptorSetSampledImages,
									  shadowMapDescriptorSlots / 2),
								  16384);
	const LunaDescriptorSetLayoutBinding spotLightShadowMapsBinding = {
		.bindingName = "Shadow Maps",
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = shadowMapSlotsAvailable,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};
	const LunaDescriptorSetLayoutCreationInfo spotLightShadowMapsDescriptorSetLayoutCreationInfo = {
		.bindingCount = 1,
		.bindings = &spotLightShadowMapsBinding,
	};
	VulkanTest(lunaCreateDescriptorSetLayout(device,
											 &spotLightShadowMapsDescriptorSetLayoutCreationInfo,
											 &spotLightShadowMapsDescriptorSetLayout),
			   "Failed to create spot light shadow maps descriptor set layout!");

	const LunaDescriptorSetLayoutBinding pointLightShadowMapsBinding = {
		.bindingName = "Shadow Maps",
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = shadowMapSlotsAvailable,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};
	const LunaDescriptorSetLayoutCreationInfo pointLightShadowMapsDescriptorSetLayoutCreationInfo = {
		.bindingCount = 1,
		.bindings = &pointLightShadowMapsBinding,
	};
	VulkanTest(lunaCreateDescriptorSetLayout(device,
											 &pointLightShadowMapsDescriptorSetLayoutCreationInfo,
											 &pointLightShadowMapsDescriptorSetLayout),
			   "Failed to create point light shadow maps descriptor set layout!");

	return true;
}

bool CreateTextureSamplers()
{
	float maxAnisotropy = 0;
	switch (GetState()->options.anisotropy)
	{
		case ANISOTROPY_2X:
			maxAnisotropy = 2;
			break;
		case ANISOTROPY_4X:
			maxAnisotropy = 4;
			break;
		case ANISOTROPY_8X:
			maxAnisotropy = 8;
			break;
		case ANISOTROPY_16X:
			maxAnisotropy = 16;
			break;
		default:
			break;
	}
	maxAnisotropy = min(maxAnisotropy, physicalDeviceProperties.limits.maxSamplerAnisotropy);
	const LunaSamplerCreationInfo linearRepeatSamplerAnisotropyCreateInfo = {
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipmapLodBias = -1.5f,
		.anisotropyEnable = maxAnisotropy == 0 ? VK_FALSE : VK_TRUE,
		.maxAnisotropy = maxAnisotropy,
		.maxLod = VK_LOD_CLAMP_NONE,
	};
	const LunaSamplerCreationInfo linearNoRepeatSamplerAnisotropyCreateInfo = {
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipmapLodBias = -1.5f,
		.anisotropyEnable = maxAnisotropy == 0 ? VK_FALSE : VK_TRUE,
		.maxAnisotropy = maxAnisotropy,
		.maxLod = VK_LOD_CLAMP_NONE,
	};

	const LunaSamplerCreationInfo linearRepeatSamplerNoAnisotropyCreateInfo = {
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipmapLodBias = -1.5f,
		.maxLod = VK_LOD_CLAMP_NONE,
	};
	const LunaSamplerCreationInfo nearestRepeatSamplerNoAnisotropyCreateInfo = {
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipmapLodBias = -1.5f,
		.maxLod = VK_LOD_CLAMP_NONE,
	};
	const LunaSamplerCreationInfo linearNoRepeatSamplerNoAnisotropyCreateInfo = {
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipmapLodBias = -1.5f,
		.maxLod = VK_LOD_CLAMP_NONE,
	};
	const LunaSamplerCreationInfo nearestNoRepeatSamplerNoAnisotropyCreateInfo = {
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipmapLodBias = -1.5f,
		.maxLod = VK_LOD_CLAMP_NONE,
	};
	const LunaSamplerCreationInfo spotLightShadowMapAtlasSamplerCreateInfo = {
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.compareEnable = VK_TRUE,
		.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
	};
	const LunaSamplerCreationInfo pointLightShadowMapAtlasSamplerCreateInfo = {
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.compareEnable = VK_TRUE,
		.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
	};

	VulkanTest(lunaCreateSampler(device,
								 &linearRepeatSamplerAnisotropyCreateInfo,
								 &textureSamplers.linearRepeatAnisotropy),
			   "Failed to create linear repeating anisotropy texture sampler!");
	VulkanTest(lunaCreateSampler(device,
								 &linearNoRepeatSamplerAnisotropyCreateInfo,
								 &textureSamplers.linearNoRepeatAnisotropy),
			   "Failed to create nearest non-repeating anisotropy texture sampler!");
	VulkanTest(lunaCreateSampler(device,
								 &linearRepeatSamplerNoAnisotropyCreateInfo,
								 &textureSamplers.linearRepeatNoAnisotropy),
			   "Failed to create linear repeating no anisotropy texture sampler!");
	VulkanTest(lunaCreateSampler(device,
								 &nearestRepeatSamplerNoAnisotropyCreateInfo,
								 &textureSamplers.nearestRepeatNoAnisotropy),
			   "Failed to create nearest repeating no anisotropy texture sampler!");
	VulkanTest(lunaCreateSampler(device,
								 &linearNoRepeatSamplerNoAnisotropyCreateInfo,
								 &textureSamplers.linearNoRepeatNoAnisotropy),
			   "Failed to create linear non-repeating no anisotropy texture sampler!");
	VulkanTest(lunaCreateSampler(device,
								 &nearestNoRepeatSamplerNoAnisotropyCreateInfo,
								 &textureSamplers.nearestNoRepeatNoAnisotropy),
			   "Failed to create nearest non-repeating no anisotropy texture sampler!");
	VulkanTest(lunaCreateSampler(device,
								 &spotLightShadowMapAtlasSamplerCreateInfo,
								 &textureSamplers.spotLightShadowMaps),
			   "Failed to create spot light shadow map atlas texture sampler!");
	VulkanTest(lunaCreateSampler(device,
								 &pointLightShadowMapAtlasSamplerCreateInfo,
								 &textureSamplers.pointLightShadowMaps),
			   "Failed to create point light shadow map atlas texture sampler!");

	ListInit(textures, LIST_UINT64);
	memset(imageAssetIdToIndexMap, -1, sizeof(*imageAssetIdToIndexMap) * MAX_TEXTURES);

	return true;
}

// TODO: Revisit this to ensure it's as it should be (update after bind flag or usage of MAX_FRAMES_IN_FLIGHT, for example)
bool CreateDescriptorSet()
{
	const uint32_t sampledImageCount = min(physicalDeviceProperties.limits.maxDescriptorSetSampledImages,
										   physicalDeviceProperties.limits.maxPerStageDescriptorSampledImages);
	LunaDescriptorPool descriptorPool = LUNA_NULL_HANDLE;
	const VkDescriptorPoolSize poolSizes[] = {
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			// I'm not sure why I need the +1 but for some reason it breaks without it
			.descriptorCount = min(MAX_TEXTURES, sampledImageCount) + 2 * min(sampledImageCount, 16384) + 4 + 1,
		},
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 3,
		},
		{
			.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
		},
	};
	const LunaDescriptorPoolCreationInfo descriptorPoolCreationInfo = {
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = 3,
		.poolSizeCount = sizeof(poolSizes) / sizeof(*poolSizes),
		.poolSizes = poolSizes,
	};
	VulkanTest(lunaCreateDescriptorPool(device, &descriptorPoolCreationInfo, &descriptorPool),
			   "Failed to create descriptor pool!");

	const LunaDescriptorSetLayout layouts[] = {
		descriptorSetLayout,
		spotLightShadowMapsDescriptorSetLayout,
		pointLightShadowMapsDescriptorSetLayout,
	};
	LunaDescriptorSet descriptorSets[] = {
		descriptorSet,
		spotLightShadowMapsDescriptorSet,
		pointLightShadowMapsDescriptorSet,
	};
	const LunaDescriptorSetAllocationInfo allocationInfo = {
		.descriptorPool = descriptorPool,
		.setLayoutCount = sizeof(layouts) / sizeof(*layouts),
		.setLayouts = layouts,
	};
	VulkanTest(lunaAllocateDescriptorSets(device, &allocationInfo, descriptorSets),
			   "Failed to allocate descriptor sets!");
	descriptorSet = descriptorSets[0];
	spotLightShadowMapsDescriptorSet = descriptorSets[1];
	pointLightShadowMapsDescriptorSet = descriptorSets[2];

	return true;
}

void WriteDescriptorSet()
{
	const LunaDescriptorBufferInfo transformMatrixBufferInfo = {
		.buffer = buffers.uniforms.camera,
	};
	const LunaWriteDescriptorSet transformMatrixWrite = {
		.descriptorSet = descriptorSet,
		.bindingName = "Camera",
		.descriptorCount = 1,
		.bufferInfos = &transformMatrixBufferInfo,
	};
	const LunaDescriptorBufferInfo lightingBufferInfo = {
		.buffer = buffers.uniforms.lighting,
	};
	const LunaWriteDescriptorSet lightingWrite = {
		.descriptorSet = descriptorSet,
		.bindingName = "Global Lighting",
		.descriptorCount = 1,
		.bufferInfos = &lightingBufferInfo,
	};
	const LunaDescriptorBufferInfo fogBufferInfo = {
		.buffer = buffers.uniforms.fog,
	};
	const LunaWriteDescriptorSet fogWrite = {
		.descriptorSet = descriptorSet,
		.bindingName = "Fog",
		.descriptorCount = 1,
		.bufferInfos = &fogBufferInfo,
	};
	lunaWriteDescriptorSets(device, 3, (LunaWriteDescriptorSet[]){transformMatrixWrite, lightingWrite, fogWrite});
}
