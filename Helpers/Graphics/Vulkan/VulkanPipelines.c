//
// Created by NBT22 on 7/7/25.
//

#include <cglm/types.h>
#include <luna/lunaPipeline.h>
#include <luna/lunaRenderPass.h>
#include <luna/lunaTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <vulkan/vulkan_core.h>
#include "../../Core/AssetLoaders/ShaderLoader.h"
#include "../../Core/AssetReader.h"
#include "VulkanHelpers.h"
#include "VulkanInternal.h"

#pragma region shared
static const VkPipelineViewportStateCreateInfo viewportState = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	.viewportCount = 1,
	.scissorCount = 1,
};

static const VkPipelineRasterizationStateCreateInfo cullingRasterizer = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	.polygonMode = VK_POLYGON_MODE_FILL,
	.cullMode = VK_CULL_MODE_BACK_BIT,
	.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
	.lineWidth = 1,
};

static const VkPipelineRasterizationStateCreateInfo nonCullingRasterizer = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	.polygonMode = VK_POLYGON_MODE_FILL,
	.cullMode = VK_CULL_MODE_NONE,
	.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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

static const VkPipelineDepthStencilStateCreateInfo depthStencilStateUnused = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
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

bool CreateUIPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/ui_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load UI vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("vulkan/ui_f"), SHADER_TYPE_FRAG, &fragShaderModule),
			   "Failed to load UI fragment shader!");

	const LunaPipelineShaderStageCreationInfo uiShaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragShaderModule,
		},
	};

	const VkVertexInputBindingDescription bindingDescription = {
		.binding = 0,
		.stride = sizeof(UiVertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	const VkVertexInputAttributeDescription attributeDescriptions[] = {
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
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = sizeof(attributeDescriptions) / sizeof(*attributeDescriptions),
		.pVertexAttributeDescriptions = attributeDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(uiShaderStages) / sizeof(*uiShaderStages),
		.shaderStages = uiShaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &cullingRasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencilStateUnused,
		.colorBlendState = &colorBlending,
		.dynamicState = &dynamicState,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&pipelineInfo, &pipelines.ui), "Failed to create UI graphics pipeline!");

	return true;
}

bool CreateViewModelPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/view_model_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load view model vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("vulkan/view_model_f"), SHADER_TYPE_FRAG, &fragShaderModule),
			   "Failed to load view model fragment shader!");

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
			.stride = sizeof(ModelVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(ModelInstanceData),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};
	const VkVertexInputAttributeDescription vertexDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ModelVertex, position),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ModelVertex, u),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelVertex, color),
		},
		{
			.location = 3,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ModelVertex, normal),
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transform) + sizeof(vec4) * 0,
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transform) + sizeof(vec4) * 1,
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transform) + sizeof(vec4) * 2,
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transform) + sizeof(vec4) * 3,
		},
		{
			.location = 8,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ModelInstanceData, textureIndex),
		},
		{
			.location = 9,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, materialColor),
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
		.rasterizationState = &cullingRasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencilStateUnused,
		.colorBlendState = &colorBlending,
		.dynamicState = &dynamicState,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&pipelineInfo, &pipelines.viewModel),
			   "Failed to create view model graphics pipeline!");

	return true;
}

bool CreateSkyPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/sky_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load sky vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("vulkan/sky_f"), SHADER_TYPE_FRAG, &fragShaderModule),
			   "Failed to load sky fragment shader!");

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

	const VkVertexInputBindingDescription bindingDescription = {
		.binding = 0,
		.stride = sizeof(SkyVertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	const VkVertexInputAttributeDescription vertexDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(SkyVertex, position),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(SkyVertex, u),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = sizeof(vertexDescriptions) / sizeof(*vertexDescriptions),
		.pVertexAttributeDescriptions = vertexDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &cullingRasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencilStateUnused,
		.colorBlendState = &colorBlending,
		.dynamicState = &dynamicState,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&pipelineInfo, &pipelines.sky), "Failed to create sky graphics pipeline!");

	return true;
}

bool CreateFloorAndCeilingPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/floor_and_ceiling_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load floor and ceiling vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("vulkan/floor_and_ceiling_f"), SHADER_TYPE_FRAG, &fragShaderModule),
			   "Failed to load floor and ceiling fragment shader!");

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

	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &cullingRasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencilStateUnused,
		.colorBlendState = &colorBlending,
		.dynamicState = &dynamicState,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&pipelineInfo, &pipelines.floorAndCeiling),
			   "Failed to create floor graphics pipeline!");

	return true;
}

bool CreateWallPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/wall_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load wall vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("vulkan/wall_f"), SHADER_TYPE_FRAG, &fragShaderModule),
			   "Failed to load wall fragment shader!");

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

	const VkVertexInputBindingDescription bindingDescription = {
		.binding = 0,
		.stride = sizeof(WallVertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	const VkVertexInputAttributeDescription vertexDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(WallVertex, position),
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
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = sizeof(vertexDescriptions) / sizeof(*vertexDescriptions),
		.pVertexAttributeDescriptions = vertexDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &nonCullingRasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencilState,
		.colorBlendState = &colorBlending,
		.dynamicState = &dynamicState,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&pipelineInfo, &pipelines.walls), "Failed to create wall graphics pipeline!");

	return true;
}

bool CreateActorWallPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/actor_wall_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to create actor vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("vulkan/actor_wall_f"), SHADER_TYPE_FRAG, &fragShaderModule),
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
			.stride = sizeof(ActorWallVertex),
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
			.offset = offsetof(ActorWallVertex, position),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorWallVertex, u),
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
		.rasterizationState = &nonCullingRasterizer,
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
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/actor_model_shaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to create actor vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("vulkan/actor_model_shaded_f"), SHADER_TYPE_FRAG, &fragShaderModule),
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
			.stride = sizeof(ModelVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(ModelInstanceData),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};
	const VkVertexInputAttributeDescription vertexDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ModelVertex, position),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ModelVertex, u),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelVertex, color),
		},
		{
			.location = 3,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ModelVertex, normal),
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transform) + sizeof(vec4) * 0,
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transform) + sizeof(vec4) * 1,
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transform) + sizeof(vec4) * 2,
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transform) + sizeof(vec4) * 3,
		},
		{
			.location = 8,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ModelInstanceData, textureIndex),
		},
		{
			.location = 9,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, materialColor),
		},
		{
			.location = 10,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, instanceColor),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(vertexDescriptions) / sizeof(*vertexDescriptions),
		.pVertexAttributeDescriptions = vertexDescriptions,
	};

	// const VkPipelineRasterizationStateCreateInfo cullingRasterizer = {
	// 	.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	// 	.polygonMode = VK_POLYGON_MODE_POINT,
	// 	.cullMode = VK_CULL_MODE_BACK_BIT,
	// 	.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
	// 	.lineWidth = 1,
	// };

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &cullingRasterizer,
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
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/actor_model_unshaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to create actor vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("vulkan/actor_model_unshaded_f"), SHADER_TYPE_FRAG, &fragShaderModule),
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
			.stride = sizeof(ModelVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(ModelInstanceData),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};
	const VkVertexInputAttributeDescription vertexDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ModelVertex, position),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ModelVertex, u),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelVertex, color),
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transform) + sizeof(vec4) * 0,
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transform) + sizeof(vec4) * 1,
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transform) + sizeof(vec4) * 2,
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transform) + sizeof(vec4) * 3,
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ModelInstanceData, textureIndex),
		},
		{
			.location = 8,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, materialColor),
		},
		{
			.location = 9,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, instanceColor),
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
		.rasterizationState = &cullingRasterizer,
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

bool CreateDebugDrawPipeline()
{
#ifdef JPH_DEBUG_RENDERER
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/debug_draw_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load debug draw vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("vulkan/debug_draw_f"), SHADER_TYPE_FRAG, &fragShaderModule),
			   "Failed to load debug draw fragment shader!");

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

	const VkVertexInputBindingDescription bindingDescription = {
		.binding = 0,
		.stride = sizeof(DebugDrawVertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	const VkVertexInputAttributeDescription vertexDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(DebugDrawVertex, position),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(DebugDrawVertex, color),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = sizeof(vertexDescriptions) / sizeof(*vertexDescriptions),
		.pVertexAttributeDescriptions = vertexDescriptions,
	};

	const VkPipelineInputAssemblyStateCreateInfo linesInputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	};
	const LunaGraphicsPipelineCreationInfo linesPipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &linesInputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &nonCullingRasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencilState,
		.colorBlendState = &colorBlending,
		.dynamicState = &dynamicState,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&linesPipelineInfo, &pipelines.debugDrawLines),
			   "Failed to create graphics pipeline for Jolt debug renderer lines!");

	const LunaGraphicsPipelineCreationInfo trianglesPipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &nonCullingRasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencilState,
		.colorBlendState = &colorBlending,
		.dynamicState = &dynamicState,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(&trianglesPipelineInfo, &pipelines.debugDrawTriangles),
			   "Failed to create graphics pipeline for Jolt debug renderer triangles!");
#endif

	return true;
}

bool CreateGraphicsPipelines()
{
	multisampling.rasterizationSamples = msaaSamples;

	// clang-format off
	return CreateUIPipeline() && CreateViewModelPipeline() && CreateSkyPipeline() && CreateFloorAndCeilingPipeline() &&
		   CreateWallPipeline() && CreateActorWallPipeline() && CreateActorModelShadedPipeline() &&
		   CreateActorModelUnshadedPipeline() && CreateDebugDrawPipeline();
	// clang-format on
}
