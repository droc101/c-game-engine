// Created by Noah on 11/23/2024.
//

#include "VulkanInternal.h"
#include <dirent.h>
#include <luna/luna.h>
#include <SDL_vulkan.h>
#include <string.h>
#include "../../../Structs/GlobalState.h"
#include "../../Core/Error.h"
#include "../../Core/Logging.h"
#include "../../Core/MathEx.h"
#include "VulkanHelpers.h"
#include "VulkanResources.h"

bool CreateInstance()
{
	uint32_t extensionCount;
	if (SDL_Vulkan_GetInstanceExtensions(vk_window, &extensionCount, NULL) == SDL_FALSE)
	{
		VulkanLogError("Failed to acquire extensions required for SDL window!\n");
		return false;
	}
	const char *extensionNames[extensionCount];
	if (SDL_Vulkan_GetInstanceExtensions(vk_window, &extensionCount, extensionNames) == SDL_FALSE)
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
	instance = lunaGetInstance();

	return true;
}

bool CreateSurface()
{
	if (SDL_Vulkan_CreateSurface(vk_window, lunaGetInstance(), &surface) == SDL_FALSE)
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
	physicalDevice.properties = lunaGetPhysicalDeviceProperties();
	return true;
}

bool CreateSwapChain()
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
		SDL_Vulkan_GetDrawableSize(vk_window, &width, &height);
		swapChainExtent.width = clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		swapChainExtent.height = clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	}

	List presentModes;
	ListCreate(&presentModes);
	if (GetState()->options.vsync)
	{
		ListAdd(&presentModes, (void *)VK_PRESENT_MODE_FIFO_KHR);
	} else
	{
		ListAddBatched(&presentModes,
					   3,
					   VK_PRESENT_MODE_MAILBOX_KHR,
					   VK_PRESENT_MODE_IMMEDIATE_KHR,
					   VK_PRESENT_MODE_FIFO_KHR);
	}

	const LunaSwapChainCreationInfo swapChainCreationInfo = {
		.surface = surface,
		.width = swapChainExtent.width,
		.height = swapChainExtent.height,
		.minImageCount = capabilities.minImageCount + 1,
		.formatCount = 2,
		.formatPriorityList = (VkSurfaceFormatKHR[]){{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
													 {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}},
		.presentModeCount = presentModes.length,
		.presentModePriorityList = (const VkPresentModeKHR *)presentModes.data,
	};

	VulkanTest(lunaCreateSwapChain(&swapChainCreationInfo), "Failed to create swap chain!");

	ListFree(&presentModes, false);

	return true;
}

bool CreateRenderPass()
{
	// TODO: Once Luna supports it, prefer using VK_FORMAT_D16_UNORM
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
	if (!(physicalDevice.properties.limits.framebufferColorSampleCounts &
		  physicalDevice.properties.limits.framebufferDepthSampleCounts &
		  msaaSamples))
	{
		ShowWarning("Invalid Settings",
					"Your GPU driver does not support the selected MSAA level!\n"
					"A fallback has been set to avoid issues.");
		while (!(physicalDevice.properties.limits.framebufferColorSampleCounts &
				 physicalDevice.properties.limits.framebufferDepthSampleCounts &
				 msaaSamples))
		{
			msaaSamples >>= 1;
			if (msaaSamples == 0)
			{
				Error("Your graphics card cannot graphics please buy a new one.");
			}
		}
	}

	const LunaSubpassCreationInfo subpassCreationInfos[] = {
		{
			.name = "Level",
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.useColorAttachment = true,
			.useDepthAttachment = true,
		},
		{
			.name = "UI",
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.useColorAttachment = true,
		},
	};
	const VkSubpassDependency dependencies[] = {
		{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		},
		{
			.srcSubpass = 0,
			.dstSubpass = 1,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		},
	};
	const LunaRenderPassCreationInfo renderPassCreationInfo = {
		.samples = msaaSamples,
		.createColorAttachment = true,
		.colorAttachmentLoadMode = LUNA_ATTACHMENT_LOAD_CLEAR,
		.createDepthAttachment = true,
		.depthAttachmentLoadMode = LUNA_ATTACHMENT_LOAD_CLEAR,
		.subpassCount = 2,
		.subpasses = subpassCreationInfos,
		.dependencyCount = 2,
		.dependencies = dependencies,
		.extent = (VkExtent3D){.width = swapChainExtent.width, .height = swapChainExtent.height, .depth = 1},
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

bool CreateGraphicsPipelines()
{
#pragma region shared
	const VkViewport viewport = {
		.width = (float)swapChainExtent.width,
		.height = (float)swapChainExtent.height,
		.maxDepth = 1,
	};
	const VkRect2D scissor = {
		.extent = swapChainExtent,
	};
	const VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	const VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.lineWidth = 1,
	};

	const VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = msaaSamples,
		.minSampleShading = 1,
	};

	const VkPipelineDepthStencilStateCreateInfo depthStencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.maxDepthBounds = 1,
	};

	const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
						  VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT |
						  VK_COLOR_COMPONENT_A_BIT,
	};
	const VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
	};

	if (sizeof(PushConstants) > physicalDevice.properties.limits.maxPushConstantsSize)
	{
		Error("Go support core 1.0 then get back to me. (Max push constant size exceeded)");
	}
	const LunaPushConstantsRange pushConstantRange = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.size = sizeof(PushConstants),
		.dataPointer = &pushConstants,
	};
	const LunaPipelineLayoutCreationInfo pipelineLayoutCreationInfo = {
		.descriptorSetLayoutCount = 1,
		.descriptorSetLayouts = &descriptorSetLayout,
		.pushConstantRangeCount = 1,
		.pushConstantsRanges = &pushConstantRange,
	};

	const VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};
#pragma endregion shared

#pragma region walls
	VkShaderModule wallVertShaderModule;
	VkShaderModule wallFragShaderModule;
	VulkanTest(CreateShaderModule(VK_VERT("Vulkan_wall"), &wallVertShaderModule), "Failed to load wall vertex shader!");
	VulkanTest(CreateShaderModule(VK_FRAG("Vulkan_wall"), &wallFragShaderModule),
			   "Failed to load wall fragment shader!");

	const VkPipelineShaderStageCreateInfo wallShaderStages[2] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = wallVertShaderModule,
			.pName = "main",
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = wallFragShaderModule,
			.pName = "main",
		},
	};

	const VkVertexInputBindingDescription wallBindingDescriptions[2] = {
		{
			.binding = 0,
			.stride = sizeof(ShadowVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(WallVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
	};
	const VkVertexInputAttributeDescription wallVertexDescriptions[5] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ShadowVertex, x),
		},
		{
			.location = 1,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(WallVertex, x),
		},
		{
			.location = 2,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(WallVertex, u),
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(WallVertex, textureIndex),
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(WallVertex, wallAngle),
		},
	};
	const VkPipelineVertexInputStateCreateInfo wallVertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 2,
		.pVertexBindingDescriptions = wallBindingDescriptions,
		.vertexAttributeDescriptionCount = 5,
		.pVertexAttributeDescriptions = wallVertexDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo wallPipelineInfo = {
		.shaderStageCount = 2,
		.shaderStages = wallShaderStages,
		.vertexInputState = &wallVertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &rasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencil,
		.colorBlendState = &colorBlending,
		.layoutCreationInfo = &pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, "Level"),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&wallPipelineInfo, &pipelines.walls),
			   "Failed to create wall graphics pipeline!");
#pragma endregion walls

#pragma region actors
	VkShaderModule actorVertShaderModule;
	VkShaderModule actorFragShaderModule;
	VulkanTest(CreateShaderModule(VK_VERT("Vulkan_actor"), &actorVertShaderModule),
			   "Failed to create actor vertex shader!");
	VulkanTest(CreateShaderModule(VK_FRAG("Vulkan_actor"), &actorFragShaderModule),
			   "Failed to create actor fragment shader!");

	const VkPipelineShaderStageCreateInfo actorShaderStages[2] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = actorVertShaderModule,
			.pName = "main",
			.pSpecializationInfo = NULL,
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = actorFragShaderModule,
			.pName = "main",
			.pSpecializationInfo = NULL,
		},
	};

	const VkVertexInputBindingDescription actorBindingDescriptions[2] = {
		{
			.binding = 0,
			.stride = sizeof(ActorVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(ActorInstanceData),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};
	const VkVertexInputAttributeDescription actorVertexDescriptions[9] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ActorVertex, x),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorVertex, u),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ActorVertex, nx),
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorInstanceData, transform) + sizeof(vec4) * 0,
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorInstanceData, transform) + sizeof(vec4) * 1,
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorInstanceData, transform) + sizeof(vec4) * 2,
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorInstanceData, transform) + sizeof(vec4) * 3,
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ActorInstanceData, textureIndex),
		},
		{
			.location = 8,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(ActorInstanceData, wallAngle),
		},
	};
	const VkPipelineVertexInputStateCreateInfo actorVertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 2,
		.pVertexBindingDescriptions = actorBindingDescriptions,
		.vertexAttributeDescriptionCount = 9,
		.pVertexAttributeDescriptions = actorVertexDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo actorPipelineInfo = {
		.shaderStageCount = 2,
		.shaderStages = actorShaderStages,
		.vertexInputState = &actorVertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &rasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencil,
		.colorBlendState = &colorBlending,
		.layoutCreationInfo = &pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, "Level"),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&actorPipelineInfo, &pipelines.actors),
			   "Failed to create actor graphics pipeline!");
#pragma endregion actors

#pragma region UI
	VkShaderModule uiVertShaderModule;
	VkShaderModule uiFragShaderModule;
	VulkanTest(CreateShaderModule(VK_VERT("Vulkan_ui"), &uiVertShaderModule), "Failed to load UI vertex shader!");
	VulkanTest(CreateShaderModule(VK_FRAG("Vulkan_ui"), &uiFragShaderModule), "Failed to load UI fragment shader!");

	const VkPipelineShaderStageCreateInfo uiShaderStages[2] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = uiVertShaderModule,
			.pName = "main",
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = uiFragShaderModule,
			.pName = "main",
		},
	};

	const VkVertexInputBindingDescription uiBindingDescription = {
		.binding = 0,
		.stride = sizeof(UiVertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	const VkVertexInputAttributeDescription uiAttributeDescriptions[4] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(UiVertex, x),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(UiVertex, u),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(UiVertex, r),
		},
		{
			.location = 3,
			.binding = 0,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(UiVertex, textureIndex),
		},
	};
	const VkPipelineVertexInputStateCreateInfo uiVertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &uiBindingDescription,
		.vertexAttributeDescriptionCount = 4,
		.pVertexAttributeDescriptions = uiAttributeDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo uiPipelineInfo = {
		.shaderStageCount = 2,
		.shaderStages = uiShaderStages,
		.vertexInputState = &uiVertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &rasterizer,
		.multisampleState = &multisampling,
		.colorBlendState = &colorBlending,
		.layoutCreationInfo = &pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, "UI"),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&uiPipelineInfo, &pipelines.ui), "Failed to create UI graphics pipeline!");
#pragma endregion UI

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
		.maxAnisotropy = 1,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.maxLod = VK_LOD_CLAMP_NONE,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
	};
	const LunaSamplerCreationInfo nearestRepeatSamplerCreateInfo = {
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = -1.5f,
		.maxAnisotropy = 1,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.maxLod = VK_LOD_CLAMP_NONE,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
	};
	const LunaSamplerCreationInfo linearNoRepeatSamplerCreateInfo = {
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipLodBias = -1.5f,
		.maxAnisotropy = 1,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.maxLod = VK_LOD_CLAMP_NONE,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
	};
	const LunaSamplerCreationInfo nearestNoRepeatSamplerCreateInfo = {
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipLodBias = -1.5f,
		.maxAnisotropy = 1,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.maxLod = VK_LOD_CLAMP_NONE,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
	};

	VulkanTest(lunaCreateSampler(&linearRepeatSamplerCreateInfo, &textureSamplers.linearRepeat),
			   "Failed to create linear repeating texture sampler!");
	VulkanTest(lunaCreateSampler(&nearestRepeatSamplerCreateInfo, &textureSamplers.nearestRepeat),
			   "Failed to create nearest repeating texture sampler!");
	VulkanTest(lunaCreateSampler(&linearNoRepeatSamplerCreateInfo, &textureSamplers.linearNoRepeat),
			   "Failed to create linear non-repeating texture sampler!");
	VulkanTest(lunaCreateSampler(&nearestNoRepeatSamplerCreateInfo, &textureSamplers.nearestNoRepeat),
			   "Failed to create nearest non-repeating texture sampler!");

	ListCreate(&textures);
	memset(imageAssetIdToIndexMap, -1, sizeof(*imageAssetIdToIndexMap) * MAX_TEXTURES);

	return true;
}

bool CreateBuffers()
{
	VulkanTest(CreateUiBuffers(), "Failed to create UI buffers!");
	VulkanTest(CreateWallBuffers(), "Failed to create wall buffers!");
	VulkanTest(CreateShadowBuffers(), "Failed to create shadow buffers!");
	VulkanTest(CreateWallActorBuffers(), "Failed to create wall actor buffers!");
	VulkanTest(CreateModelActorBuffers(), "Failed to create model actor buffers!");
	VulkanTest(CreateRoofBuffers(), "Failed to create roof buffers!");

	return true;
}

bool CreateDescriptorPool()
{
	const VkDescriptorPoolSize poolSizes[] = {
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT,
		},
	};
	const LunaDescriptorPoolCreationInfo descriptorPoolCreationInfo = {
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.maxSets = MAX_FRAMES_IN_FLIGHT,
		.poolSizeCount = 1,
		.poolSizes = poolSizes,
	};
	VulkanTest(lunaCreateDescriptorPool(&descriptorPoolCreationInfo, &descriptorPool),
			   "Failed to create descriptor pool!");

	return true;
}

bool CreateDescriptorSets()
{
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

	// VkDescriptorImageInfo imageInfo[textures.length];
	// for (size_t textureIndex = 0; textureIndex < textures.length; textureIndex++)
	// {
	// 	imageInfo[textureIndex] = (VkDescriptorImageInfo){
	// 		.sampler = textureSamplers.nearestRepeat,
	// 		.imageView = ListGet(texturesImageView, textureIndex),
	// 		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	// 	};
	// }
	// for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	// {
	// 	if (textures.length == 0)
	// 	{
	// 		break;
	// 	}
	// 	const VkWriteDescriptorSet writeDescriptor = {
	// 		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	// 		.pNext = NULL,
	// 		.dstSet = descriptorSets[i],
	// 		.dstBinding = 0,
	// 		.dstArrayElement = 0,
	// 		.descriptorCount = textures.length,
	// 		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	// 		.pImageInfo = imageInfo,
	// 		.pBufferInfo = NULL,
	// 		.pTexelBufferView = NULL,
	// 	};
	// 	vkUpdateDescriptorSets(device, 1, &writeDescriptor, 0, NULL);
	// }

	return true;
}
/*
bool CreateCommandBuffers()
{
	const VkCommandBufferAllocateInfo graphicsAllocateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = graphicsCommandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT,
	};
	VulkanTest(vkAllocateCommandBuffers(device, &graphicsAllocateInfo, commandBuffers),
			   "Failed to allocate Vulkan graphics command buffers!");

	const VkCommandBufferAllocateInfo transferAllocateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = transferCommandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	VulkanTest(vkAllocateCommandBuffers(device, &transferAllocateInfo, &transferCommandBuffer),
			   "Failed to allocate Vulkan transfer command buffer!");

	const VkFenceCreateInfo fenceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};
	VulkanTest(vkCreateFence(device, &fenceCreateInfo, NULL, &transferBufferFence),
			   "Failed to create Vulkan transfer buffer fence!");

	return true;
}

bool CreateSyncObjects()
{
	const VkSemaphoreCreateInfo semaphoreInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
	};

	const VkFenceCreateInfo fenceInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VulkanTest(vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphores[i]),
				   "Failed to create Vulkan semaphores!");

		VulkanTest(vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphores[i]),
				   "Failed to create Vulkan semaphores!");

		VulkanTest(vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[i]), "Failed to create Vulkan semaphores!");
	}

	return true;
}
*/
