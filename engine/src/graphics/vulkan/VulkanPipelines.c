//
// Created by NBT22 on 7/7/25.
//

#include <cglm/types.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/ShaderLoader.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanInternal.h>
#include <luna/luna.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <vulkan/vulkan_core.h>

// TODO: This probably won't change much since pipelines are really just a lot of boilerplate,
//  but make sure to go through and add documentation as well as ensuring there aren't any cut corners left in.

#pragma region shared
static const VkPipelineViewportStateCreateInfo VIEWPORT_STATE = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	.viewportCount = 1,
	.scissorCount = 1,
};

static const VkPipelineRasterizationStateCreateInfo RASTERIZER = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	.polygonMode = VK_POLYGON_MODE_FILL,
	.cullMode = VK_CULL_MODE_BACK_BIT,
	.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
	.lineWidth = 1,
};

static VkPipelineMultisampleStateCreateInfo multisampling = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	.minSampleShading = 1,
};

static const VkPipelineDepthStencilStateCreateInfo DEPTH_STENCIL_STATE = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	.depthTestEnable = VK_TRUE,
	.depthWriteEnable = VK_TRUE,
	.depthCompareOp = VK_COMPARE_OP_LESS,
	.maxDepthBounds = 1,
};

static const VkPipelineDepthStencilStateCreateInfo DEPTH_STENCIL_STATE_UNUSED = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
};

static const VkPipelineColorBlendAttachmentState COLOR_BLEND_ATTACHMENT = {
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
static const VkPipelineColorBlendStateCreateInfo COLOR_BLENDING = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
	.attachmentCount = 1,
	.pAttachments = &COLOR_BLEND_ATTACHMENT,
};

static LunaPipelineLayoutCreationInfo pipelineLayoutCreationInfo = {
	.descriptorSetLayoutCount = 1,
};

static const VkPipelineInputAssemblyStateCreateInfo INPUT_ASSEMBLY = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
};

static const VkPipelineDynamicStateCreateInfo DYNAMIC_STATE = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
	.dynamicStateCount = 2,
	.pDynamicStates = (VkDynamicState[]){VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR},
};

static LunaShaderModule modelShadedFragShaderModule = LUNA_NULL_HANDLE;
static LunaShaderModule modelUnshadedFragShaderModule = LUNA_NULL_HANDLE;
#pragma endregion shared

static inline bool CreateUIPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/ui_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load UI vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("vulkan/ui_f"), SHADER_TYPE_FRAG, &fragShaderModule),
			   "Failed to load UI fragment shader!");

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
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.rasterizationState = &RASTERIZER,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE_UNUSED,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(device, &pipelineInfo, &pipelines.ui),
			   "Failed to create UI graphics pipeline!");

	return true;
}

// TODO: This might be identical to the unshaded map pipeline now that normals aren't around. Check if it is
static inline bool CreateShadedMapPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/map_shaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load shaded map vertex shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = modelUnshadedFragShaderModule,
		},
	};

	const VkVertexInputBindingDescription bindingDescriptions[] = {
		{
			.binding = 0,
			.stride = sizeof(MapVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(uint32_t),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};
	const VkVertexInputAttributeDescription attributeDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(MapVertex, position),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(MapVertex, uv),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(MapVertex, lightmapUv),
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = 0,
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(attributeDescriptions) / sizeof(*attributeDescriptions),
		.pVertexAttributeDescriptions = attributeDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.rasterizationState = &RASTERIZER,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(device, &pipelineInfo, &pipelines.shadedMap),
			   "Failed to create shaded map graphics pipeline!");

	return true;
}

static inline bool CreateUnshadedMapPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/map_unshaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load unshaded map vertex shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = modelUnshadedFragShaderModule,
		},
	};

	const VkVertexInputBindingDescription bindingDescriptions[] = {
		{
			.binding = 0,
			.stride = sizeof(MapVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(uint32_t),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};
	const VkVertexInputAttributeDescription attributeDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(MapVertex, position),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(MapVertex, uv),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(MapVertex, lightmapUv),
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = 0,
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(attributeDescriptions) / sizeof(*attributeDescriptions),
		.pVertexAttributeDescriptions = attributeDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.rasterizationState = &RASTERIZER,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(device, &pipelineInfo, &pipelines.unshadedMap),
			   "Failed to create unshaded map graphics pipeline!");

	return true;
}

static inline bool CreateSkyPipeline()
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

	const VkVertexInputBindingDescription bindingDescriptions[] = {
		{
			.binding = 0,
			.stride = sizeof(SkyVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
	};
	const VkVertexInputAttributeDescription attributeDescriptions[] = {
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
			.offset = offsetof(SkyVertex, uv),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(attributeDescriptions) / sizeof(*attributeDescriptions),
		.pVertexAttributeDescriptions = attributeDescriptions,
	};

	const LunaPushConstantsRange pushConstantsRange = {
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.size = sizeof(uint32_t),
		.dataPointer = &skyTextureIndex,
	};
	const LunaPipelineLayoutCreationInfo skyPipelineLayoutCreationInfo = {
		.descriptorSetLayoutCount = 1,
		.descriptorSetLayouts = pipelineLayoutCreationInfo.descriptorSetLayouts,
		.pushConstantRangeCount = 1,
		.pushConstantsRanges = &pushConstantsRange,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.rasterizationState = &RASTERIZER,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE_UNUSED,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = skyPipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(device, &pipelineInfo, &pipelines.sky),
			   "Failed to create sky graphics pipeline!");

	return true;
}

static inline bool CreateShadedViewmodelPipeline()
{
	// Layout of textureIndex and materialColor is assumed to be a known promise, so ensure that is true
	static_assert(offsetof(ModelInstanceData, textureIndex) ==
				  offsetof(ModelInstanceData, materialColor) + SizeofMember(ModelInstanceData, materialColor));

	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/viewmodel_shaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load shaded viewmodel vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("vulkan/viewmodel_shaded_f"), SHADER_TYPE_FRAG, &fragShaderModule),
			   "Failed to load shaded viewmodel fragment shader!");

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
	const VkVertexInputAttributeDescription attributeDescriptions[] = {
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
			.offset = offsetof(ModelVertex, uv),
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
			.offset = offsetof(ModelInstanceData, transformMatrix) + sizeof(vec4) * 0,
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transformMatrix) + sizeof(vec4) * 1,
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transformMatrix) + sizeof(vec4) * 2,
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transformMatrix) + sizeof(vec4) * 3,
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
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ModelInstanceData, textureIndex),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(attributeDescriptions) / sizeof(*attributeDescriptions),
		.pVertexAttributeDescriptions = attributeDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.rasterizationState = &RASTERIZER,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE_UNUSED,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(device, &pipelineInfo, &pipelines.shadedViewmodel),
			   "Failed to create shaded viewmodel graphics pipeline!");

	return true;
}

static inline bool CreateUnshadedViewmodelPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/viewmodel_unshaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load unshaded viewmodel vertex shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = modelUnshadedFragShaderModule,
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
	const VkVertexInputAttributeDescription attributeDescriptions[] = {
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
			.offset = offsetof(ModelVertex, uv),
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
			.offset = offsetof(ModelInstanceData, transformMatrix) + sizeof(vec4) * 0,
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transformMatrix) + sizeof(vec4) * 1,
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transformMatrix) + sizeof(vec4) * 2,
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, transformMatrix) + sizeof(vec4) * 3,
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ModelInstanceData, materialColor),
		},
		{
			.location = 8,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ModelInstanceData, textureIndex),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(attributeDescriptions) / sizeof(*attributeDescriptions),
		.pVertexAttributeDescriptions = attributeDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.rasterizationState = &RASTERIZER,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE_UNUSED,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(device, &pipelineInfo, &pipelines.unshadedViewmodel),
			   "Failed to create unshaded viewmodel graphics pipeline!");

	return true;
}

static inline bool CreateShadedActorModelPipeline()
{
	// Layout of textureIndex and materialColor is assumed to be a known promise, so ensure that is true
	static_assert(offsetof(ActorModelInstanceData, textureIndex) ==
				  offsetof(ActorModelInstanceData, materialColor) +
						  SizeofMember(ActorModelInstanceData, materialColor));

	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/actor_model_shaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load shaded actor model vertex shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = modelShadedFragShaderModule,
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
			.stride = sizeof(ActorModelInstanceData),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};
	const VkVertexInputAttributeDescription attributeDescriptions[] = {
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
			.offset = offsetof(ModelVertex, uv),
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
			.offset = offsetof(ActorModelInstanceData, transformMatrix) + sizeof(vec4) * 0,
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transformMatrix) + sizeof(vec4) * 1,
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transformMatrix) + sizeof(vec4) * 2,
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transformMatrix) + sizeof(vec4) * 3,
		},
		{
			.location = 8,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, modColor),
		},
		{
			.location = 9,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, materialColor),
		},
		{
			.location = 10,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ActorModelInstanceData, textureIndex),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(attributeDescriptions) / sizeof(*attributeDescriptions),
		.pVertexAttributeDescriptions = attributeDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.rasterizationState = &RASTERIZER,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE_UNUSED,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(device, &pipelineInfo, &pipelines.shadedActorModel),
			   "Failed to create shaded actor model graphics pipeline!");

	return true;
}

static inline bool CreateUnshadedActorModelPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("vulkan/actor_model_unshaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load unshaded actor model vertex shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = modelUnshadedFragShaderModule,
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
			.stride = sizeof(ActorModelInstanceData),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};
	const VkVertexInputAttributeDescription attributeDescriptions[] = {
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
			.offset = offsetof(ModelVertex, uv),
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
			.offset = offsetof(ActorModelInstanceData, transformMatrix) + sizeof(vec4) * 0,
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transformMatrix) + sizeof(vec4) * 1,
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transformMatrix) + sizeof(vec4) * 2,
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transformMatrix) + sizeof(vec4) * 3,
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, modColor),
		},
		{
			.location = 8,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, materialColor),
		},
		{
			.location = 9,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ActorModelInstanceData, textureIndex),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(attributeDescriptions) / sizeof(*attributeDescriptions),
		.pVertexAttributeDescriptions = attributeDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.rasterizationState = &RASTERIZER,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE_UNUSED,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(device, &pipelineInfo, &pipelines.unshadedActorModel),
			   "Failed to create unshaded actor model graphics pipeline!");

	return true;
}

static inline bool CreateDebugDrawPipeline()
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
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &COLOR_BLENDING,
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
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &viewportState,
		.rasterizationState = &nonCullingRasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &COLOR_BLENDING,
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
	pipelineLayoutCreationInfo.descriptorSetLayouts = &descriptorSetLayout;

	VulkanTest(CreateShaderModule(SHADER("vulkan/model_shaded_f"), SHADER_TYPE_FRAG, &modelShadedFragShaderModule),
			   "Failed to load shaded model fragment shader!");
	VulkanTest(CreateShaderModule(SHADER("vulkan/model_unshaded_f"), SHADER_TYPE_FRAG, &modelUnshadedFragShaderModule),
			   "Failed to load unshaded model fragment shader!");

	return CreateUIPipeline() &&
		   CreateShadedMapPipeline() &&
		   CreateUnshadedMapPipeline() &&
		   CreateSkyPipeline() &&
		   CreateShadedViewmodelPipeline() &&
		   CreateUnshadedViewmodelPipeline() &&
		   CreateShadedActorModelPipeline() &&
		   CreateUnshadedActorModelPipeline() &&
		   CreateDebugDrawPipeline();
}
