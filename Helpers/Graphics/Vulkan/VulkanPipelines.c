//
// Created by NBT22 on 7/7/25.
//

#include <assert.h>
#include <luna/luna.h>
#include "VulkanHelpers.h"
#include "VulkanInternal.h"

#pragma region shared
static const VkPipelineViewportStateCreateInfo viewportState = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	.viewportCount = 1,
	.scissorCount = 1,
};

static const VkPipelineRasterizationStateCreateInfo rasterizer = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	.polygonMode = VK_POLYGON_MODE_FILL,
	.cullMode = VK_CULL_MODE_NONE,
	.frontFace = VK_FRONT_FACE_CLOCKWISE,
	.lineWidth = 1,
};

static VkPipelineMultisampleStateCreateInfo multisampling = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	.minSampleShading = 1,
};

static const VkPipelineDepthStencilStateCreateInfo depthStencilState = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	.depthTestEnable = VK_TRUE,
	.depthWriteEnable = VK_TRUE,
	.depthCompareOp = VK_COMPARE_OP_LESS,
	.maxDepthBounds = 1,
};

static const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
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
static const VkPipelineColorBlendStateCreateInfo colorBlending = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
	.attachmentCount = 1,
	.pAttachments = &colorBlendAttachment,
};
static const LunaPushConstantsRange pushConstantRange = {
	.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
	.size = sizeof(PushConstants),
	.dataPointer = &pushConstants,
};
static const LunaPipelineLayoutCreationInfo pipelineLayoutCreationInfo = {
	.descriptorSetLayoutCount = 1,
	.descriptorSetLayouts = &descriptorSetLayout,
	.pushConstantRangeCount = 1,
	.pushConstantsRanges = &pushConstantRange,
};

static const VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
};

static const VkPipelineDynamicStateCreateInfo dynamicState = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
	.dynamicStateCount = 2,
	.pDynamicStates = (VkDynamicState[]){VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR},
};
#pragma endregion shared

bool CreateWallPipeline()
{
	LunaShaderModule wallVertShaderModule;
	LunaShaderModule wallFragShaderModule;
	VulkanTest(CreateShaderModule(VK_VERT("Vulkan_wall"), &wallVertShaderModule), "Failed to load wall vertex shader!");
	VulkanTest(CreateShaderModule(VK_FRAG("Vulkan_wall"), &wallFragShaderModule),
			   "Failed to load wall fragment shader!");

	const LunaPipelineShaderStageCreationInfo wallShaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = wallVertShaderModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = wallFragShaderModule,
		},
	};

	const VkVertexInputBindingDescription wallBindingDescriptions[] = {
		{
			.binding = 0,
			.stride = sizeof(WallVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
	};
	const VkVertexInputAttributeDescription wallVertexDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(WallVertex, x),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(WallVertex, u),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(WallVertex, textureIndex),
		},
		{
			.location = 3,
			.binding = 0,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(WallVertex, wallAngle),
		},
	};
	const VkPipelineVertexInputStateCreateInfo wallVertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(wallBindingDescriptions) / sizeof(*wallBindingDescriptions),
		.pVertexBindingDescriptions = wallBindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(wallVertexDescriptions) / sizeof(*wallVertexDescriptions),
		.pVertexAttributeDescriptions = wallVertexDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo wallPipelineInfo = {
		.shaderStageCount = sizeof(wallShaderStages) / sizeof(*wallShaderStages),
		.shaderStages = wallShaderStages,
		.vertexInputState = &wallVertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &rasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencilState,
		.colorBlendState = &colorBlending,
		.dynamicState = &dynamicState,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&wallPipelineInfo, &pipelines.walls),
			   "Failed to create wall graphics pipeline!");

	return true;
}

bool CreateActorWallPipeline()
{
	LunaShaderModule vertShaderModule;
	LunaShaderModule fragShaderModule;
	VulkanTest(CreateShaderModule(VK_VERT("Vulkan_actor_wall"), &vertShaderModule),
			   "Failed to create actor vertex shader!");
	VulkanTest(CreateShaderModule(VK_FRAG("Vulkan_actor_wall"), &fragShaderModule),
			   "Failed to create actor fragment shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragShaderModule,
		},
	};

	const VkVertexInputBindingDescription bindingDescriptions[] = {
		{
			.binding = 0,
			.stride = sizeof(ActorVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(ActorWallInstanceData),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};
	const VkVertexInputAttributeDescription vertexDescriptions[] = {
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
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, transform) + sizeof(vec4) * 0,
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, transform) + sizeof(vec4) * 1,
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, transform) + sizeof(vec4) * 2,
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, transform) + sizeof(vec4) * 3,
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ActorWallInstanceData, textureIndex),
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, wallAngle),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(vertexDescriptions) / sizeof(*vertexDescriptions),
		.pVertexAttributeDescriptions = vertexDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &rasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencilState,
		.colorBlendState = &colorBlending,
		.dynamicState = &dynamicState,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&pipelineInfo, &pipelines.actorWalls),
			   "Failed to create actor walls pipeline!");

	return true;
}

bool CreateActorModelShadedPipeline()
{
	LunaShaderModule vertShaderModule;
	LunaShaderModule fragShaderModule;
	VulkanTest(CreateShaderModule(VK_VERT("Vulkan_actor_model_shaded"), &vertShaderModule),
			   "Failed to create actor vertex shader!");
	VulkanTest(CreateShaderModule(VK_FRAG("Vulkan_actor_model_shaded"), &fragShaderModule),
			   "Failed to create actor fragment shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragShaderModule,
		},
	};

	const VkVertexInputBindingDescription bindingDescriptions[] = {
		{
			.binding = 0,
			.stride = sizeof(ActorModelVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(ActorModelInstanceData),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};
	const VkVertexInputAttributeDescription vertexDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ActorModelVertex, x),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorModelVertex, u),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ActorModelVertex, nx),
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transform) + sizeof(vec4) * 0,
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transform) + sizeof(vec4) * 1,
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transform) + sizeof(vec4) * 2,
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transform) + sizeof(vec4) * 3,
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ActorModelInstanceData, textureIndex),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(vertexDescriptions) / sizeof(*vertexDescriptions),
		.pVertexAttributeDescriptions = vertexDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &rasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencilState,
		.colorBlendState = &colorBlending,
		.dynamicState = &dynamicState,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&pipelineInfo, &pipelines.shadedActorModels),
			   "Failed to create shaded actor models pipeline!");

	return true;
}

bool CreateActorModelUnshadedPipeline()
{
	LunaShaderModule vertShaderModule;
	LunaShaderModule fragShaderModule;
	VulkanTest(CreateShaderModule(VK_VERT("Vulkan_actor_model_unshaded"), &vertShaderModule),
			   "Failed to create actor vertex shader!");
	VulkanTest(CreateShaderModule(VK_FRAG("Vulkan_actor_model_unshaded"), &fragShaderModule),
			   "Failed to create actor fragment shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragShaderModule,
		},
	};

	const VkVertexInputBindingDescription bindingDescriptions[] = {
		{
			.binding = 0,
			.stride = sizeof(ActorModelVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(ActorModelInstanceData),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};
	const VkVertexInputAttributeDescription vertexDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ActorModelVertex, x),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorModelVertex, u),
		},
		{
			.location = 2,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transform) + sizeof(vec4) * 0,
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transform) + sizeof(vec4) * 1,
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transform) + sizeof(vec4) * 2,
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transform) + sizeof(vec4) * 3,
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ActorModelInstanceData, textureIndex),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(vertexDescriptions) / sizeof(*vertexDescriptions),
		.pVertexAttributeDescriptions = vertexDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &rasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencilState,
		.colorBlendState = &colorBlending,
		.dynamicState = &dynamicState,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&pipelineInfo, &pipelines.unshadedActorModels),
			   "Failed to create unshaded actor models pipeline!");

	return true;
}

bool CreateUIPipeline()
{
	LunaShaderModule uiVertShaderModule;
	LunaShaderModule uiFragShaderModule;
	VulkanTest(CreateShaderModule(VK_VERT("Vulkan_ui"), &uiVertShaderModule), "Failed to load UI vertex shader!");
	VulkanTest(CreateShaderModule(VK_FRAG("Vulkan_ui"), &uiFragShaderModule), "Failed to load UI fragment shader!");

	const LunaPipelineShaderStageCreationInfo uiShaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = uiVertShaderModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = uiFragShaderModule,
		},
	};

	const VkVertexInputBindingDescription uiBindingDescription = {
		.binding = 0,
		.stride = sizeof(UiVertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	const VkVertexInputAttributeDescription uiAttributeDescriptions[] = {
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
		.vertexAttributeDescriptionCount = sizeof(uiAttributeDescriptions) / sizeof(*uiAttributeDescriptions),
		.pVertexAttributeDescriptions = uiAttributeDescriptions,
	};

	const VkPipelineDepthStencilStateCreateInfo uiDepthStencilState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	};

	const LunaGraphicsPipelineCreationInfo uiPipelineInfo = {
		.shaderStageCount = sizeof(uiShaderStages) / sizeof(*uiShaderStages),
		.shaderStages = uiShaderStages,
		.vertexInputState = &uiVertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &rasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &uiDepthStencilState,
		.colorBlendState = &colorBlending,
		.dynamicState = &dynamicState,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&uiPipelineInfo, &pipelines.ui), "Failed to create UI graphics pipeline!");

	return true;
}

bool CreateGraphicsPipelines()
{
	assert(sizeof(PushConstants) <= physicalDeviceLimits.maxPushConstantsSize);
	multisampling.rasterizationSamples = msaaSamples;

	return CreateWallPipeline() &&
		   CreateActorWallPipeline() &&
		   CreateActorModelShadedPipeline() &&
		   CreateActorModelUnshadedPipeline() &&
		   CreateUIPipeline();
}
