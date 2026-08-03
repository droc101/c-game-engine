//
// Created by NBT22 on 7/7/25.
//

#include <assert.h>
#include <cglm/types.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/assets/ShaderLoader.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/graphics/vulkan/VulkanInternal.h>
#include <engine/structs/Map.h>
#include <luna/luna.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#ifdef JPH_DEBUG_RENDERER
// Needed to #ifndef JPH_DEBUG_RENDERER_WIREFRAME
#include <engine/debug/JoltDebugRenderer.h>
#endif

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
#ifdef JPH_DEBUG_RENDERER
#ifndef JPH_DEBUG_RENDERER_WIREFRAME
	.depthBiasEnable = VK_TRUE,
	.depthBiasConstantFactor = 10,
	.depthBiasSlopeFactor = 10,
#endif
#endif
	.lineWidth = 1,
};

static const VkPipelineRasterizationStateCreateInfo SHADOW_MAP_RASTERIZER = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	.polygonMode = VK_POLYGON_MODE_FILL,
	.cullMode = VK_CULL_MODE_BACK_BIT,
	.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
	.depthBiasEnable = VK_TRUE,
	.depthBiasConstantFactor = 2.0f,
	.depthBiasSlopeFactor = 2.0f,
	.lineWidth = 1,
};

static const VkPipelineMultisampleStateCreateInfo MULTISAMPLING_DISABLED = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	.minSampleShading = 1,
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
	.blendEnable = VK_FALSE,
	.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT,
};
static const VkPipelineColorBlendStateCreateInfo COLOR_BLENDING = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
	.attachmentCount = 1,
	.pAttachments = &COLOR_BLEND_ATTACHMENT,
};

static const VkPipelineColorBlendStateCreateInfo SHADOW_MAP_COLOR_BLENDING = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
};

static LunaPipelineLayoutCreationInfo pipelineLayoutCreationInfo = {
	.descriptorSetLayoutCount = 1,
};

static LunaPushConstantsRange shadowMapPushConstantRange = {
	.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
	.size = sizeof(uint32_t),
	.dataPointerOffset = 0,
};
static LunaPipelineLayoutCreationInfo shadowMapPipelineLayoutCreationInfo = {
	.descriptorSetLayoutCount = 1,
	.pushConstantRangeCount = 1,
	.pushConstantsRanges = &shadowMapPushConstantRange,
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
	VulkanTest(CreateShaderModule(SHADER("ui_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load UI vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("ui_f"), SHADER_TYPE_FRAG, &fragShaderModule),
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

	static const VkPipelineColorBlendAttachmentState UI_PIPELINE_COLOR_BLEND_ATTACHMENT = {
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
	static const VkPipelineColorBlendStateCreateInfo UI_PIPELINE_COLOR_BLENDING = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &UI_PIPELINE_COLOR_BLEND_ATTACHMENT,
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
		.colorBlendState = &UI_PIPELINE_COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, NULL),
										  &pipelines.ui),
			   "Failed to create UI graphics pipeline!");

	return true;
}

static inline bool CreateShadedMapPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("map_shaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load shaded map vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("map_shaded_f"), SHADER_TYPE_FRAG, &fragShaderModule),
			   "Failed to load shaded map fragment shader!");

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
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(MapVertex, normal),
		},
		{
			.location = 4,
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
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, NULL),
										  &pipelines.shadedMap),
			   "Failed to create shaded map graphics pipeline!");

	return true;
}

static inline bool CreateUnshadedMapPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("map_unshaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
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
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, NULL),
										  &pipelines.unshadedMap),
			   "Failed to create unshaded map graphics pipeline!");

	return true;
}

static inline bool CreateSkyPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("sky_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load sky vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("sky_f"), SHADER_TYPE_FRAG, &fragShaderModule),
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
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, NULL),
										  &pipelines.sky),
			   "Failed to create sky graphics pipeline!");

	return true;
}

static inline bool CreateShadedModelPipeline()
{
	// Layout of textureIndex and materialColor is assumed to be a known promise, so ensure that is true
	static_assert(offsetof(ModelInstanceData, textureIndex) ==
				  offsetof(ModelInstanceData, materialColor) + SizeofMember(ModelInstanceData, materialColor));

	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("model_shaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load shaded model vertex shader!");

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
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, NULL),
										  &pipelines.shadedModel),
			   "Failed to create shaded model graphics pipeline!");

	return true;
}

static inline bool CreateUnshadedModelPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("model_unshaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load unshaded model vertex shader!");

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
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, NULL),
										  &pipelines.unshadedModel),
			   "Failed to create unshaded model graphics pipeline!");

	return true;
}

static inline bool CreateShadedActorModelPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("actor_model_shaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
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
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, NULL),
										  &pipelines.shadedActorModel),
			   "Failed to create shaded actor model graphics pipeline!");

	return true;
}

static inline bool CreateUnshadedActorModelPipeline()
{
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("actor_model_unshaded_v"), SHADER_TYPE_VERT, &vertShaderModule),
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
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, NULL),
										  &pipelines.unshadedActorModel),
			   "Failed to create unshaded actor model graphics pipeline!");

	return true;
}

static inline bool CreateActorWallPipelines()
{
	LunaShaderModule shadedVertModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("actor_wall_shaded_v"), SHADER_TYPE_VERT, &shadedVertModule),
			   "Failed to load shaded actor wall vertex shader!");
	LunaShaderModule unshadedVertModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("actor_wall_unshaded_v"), SHADER_TYPE_VERT, &unshadedVertModule),
			   "Failed to load unshaded actor wall vertex shader!");

	const LunaPipelineShaderStageCreationInfo shadedShaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = shadedVertModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = modelShadedFragShaderModule,
		},
	};
	const LunaPipelineShaderStageCreationInfo unshadedShaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = unshadedVertModule,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = modelUnshadedFragShaderModule,
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
	const VkVertexInputAttributeDescription attributeDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorWallVertex, position),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorWallVertex, uv),
		},
		{
			.location = 2,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, position),
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, scale),
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, axis),
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, centerOffset),
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, rotationQuat),
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ActorWallInstanceData, textureIndex),
		},
		{
			.location = 8,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, uvScale),
		},
		{
			.location = 9,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, uvOffset),
		},
		{
			.location = 10,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, modColor),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(attributeDescriptions) / sizeof(*attributeDescriptions),
		.pVertexAttributeDescriptions = attributeDescriptions,
	};

	const LunaGraphicsPipelineCreationInfo shadedPipelineInfo = {
		.shaderStageCount = sizeof(shadedShaderStages) / sizeof(*shadedShaderStages),
		.shaderStages = shadedShaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.rasterizationState = &RASTERIZER,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &shadedPipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, NULL),
										  &pipelines.shadedActorWall),
			   "Failed to create shaded actor wall graphics pipeline!");

	const LunaGraphicsPipelineCreationInfo unshadedPipelineInfo = {
		.shaderStageCount = sizeof(unshadedShaderStages) / sizeof(*unshadedShaderStages),
		.shaderStages = unshadedShaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.rasterizationState = &RASTERIZER,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &unshadedPipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, NULL),
										  &pipelines.unshadedActorWall),
			   "Failed to create unshaded actor wall graphics pipeline!");

	return true;
}

static inline bool CreateDebugDrawPipeline()
{
#ifdef JPH_DEBUG_RENDERER
	LunaShaderModule vertShaderModule = LUNA_NULL_HANDLE;
	LunaShaderModule fragShaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("debug_draw_v"), SHADER_TYPE_VERT, &vertShaderModule),
			   "Failed to load debug draw vertex shader!");
	VulkanTest(CreateShaderModule(SHADER("debug_draw_f"), SHADER_TYPE_FRAG, &fragShaderModule),
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

	static const VkPipelineRasterizationStateCreateInfo DEBUG_DRAW_RASTERIZER = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.lineWidth = 1,
	};
	static const VkPipelineInputAssemblyStateCreateInfo LINES_INPUT_ASSEMBLY = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	};
	const LunaGraphicsPipelineCreationInfo linesPipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &LINES_INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.rasterizationState = &DEBUG_DRAW_RASTERIZER,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(device, &linesPipelineInfo, &pipelines.debugDrawLines),
			   "Failed to create graphics pipeline for Jolt debug renderer lines!");

	const LunaGraphicsPipelineCreationInfo trianglesPipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.rasterizationState = &DEBUG_DRAW_RASTERIZER,
		.multisampleState = &multisampling,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = pipelineLayoutCreationInfo,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, NULL),
	};
	VulkanTest(lunaCreateGraphicsPipeline(device, &trianglesPipelineInfo, &pipelines.debugDrawTriangles),
			   "Failed to create graphics pipeline for Jolt debug renderer triangles!");
#endif

	return true;
}

static inline VkResult CreateMapShadowMapPipeline()
{
	if (pipelines.shadowMaps.map != LUNA_NULL_HANDLE)
	{
		lunaDestroyGraphicsPipeline(device, pipelines.shadowMaps.map);
	}

	LunaShaderModule shaderModule = LUNA_NULL_HANDLE;
	VulkanTestReturnResult(CreateShaderModule(SHADER("map_shadow_maps_v"), SHADER_TYPE_VERT, &shaderModule),
						   "Failed to load map shadow maps vertex shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = shaderModule,
		},
	};

	const VkVertexInputBindingDescription bindingDescriptions[] = {
		{
			.binding = 0,
			.stride = sizeof(MapVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
	};
	const VkVertexInputAttributeDescription attributeDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(MapVertex, position),
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
		.rasterizationState = &SHADOW_MAP_RASTERIZER,
		.multisampleState = &MULTISAMPLING_DISABLED,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &SHADOW_MAP_COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = shadowMapPipelineLayoutCreationInfo,
	};
	VulkanTestReturnResult(lunaCreateGraphicsPipelineWithVkRenderPass(device,
																	  &pipelineInfo,
																	  shadowMapRenderPass,
																	  0,
																	  &pipelines.shadowMaps.map),
						   "Failed to create map shadow map graphics pipeline!");

	return VK_SUCCESS;
}

static inline VkResult CreateModelActorShadowMapPipeline()
{
	if (pipelines.shadowMaps.modelActors != LUNA_NULL_HANDLE)
	{
		lunaDestroyGraphicsPipeline(device, pipelines.shadowMaps.modelActors);
	}

	LunaShaderModule shaderModule = LUNA_NULL_HANDLE;
	VulkanTestReturnResult(CreateShaderModule(SHADER("model_actor_shadow_maps_v"), SHADER_TYPE_VERT, &shaderModule),
						   "Failed to load model actor shadow maps vertex shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = shaderModule,
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
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transformMatrix) + sizeof(vec4) * 0,
		},
		{
			.location = 2,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transformMatrix) + sizeof(vec4) * 1,
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transformMatrix) + sizeof(vec4) * 2,
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorModelInstanceData, transformMatrix) + sizeof(vec4) * 3,
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
		.rasterizationState = &SHADOW_MAP_RASTERIZER,
		.multisampleState = &MULTISAMPLING_DISABLED,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &SHADOW_MAP_COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = shadowMapPipelineLayoutCreationInfo,
	};
	VulkanTestReturnResult(lunaCreateGraphicsPipelineWithVkRenderPass(device,
																	  &pipelineInfo,
																	  shadowMapRenderPass,
																	  0,
																	  &pipelines.shadowMaps.modelActors),
						   "Failed to create model actor shadow map graphics pipeline!");

	return VK_SUCCESS;
}

static inline VkResult CreateWallActorShadowMapPipeline()
{
	if (pipelines.shadowMaps.wallActors != LUNA_NULL_HANDLE)
	{
		lunaDestroyGraphicsPipeline(device, pipelines.shadowMaps.wallActors);
	}

	LunaShaderModule shaderModule = LUNA_NULL_HANDLE;
	VulkanTestReturnResult(CreateShaderModule(SHADER("wall_actor_shadow_maps_v"), SHADER_TYPE_VERT, &shaderModule),
						   "Failed to load wall actor shadow maps vertex shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = shaderModule,
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
	const VkVertexInputAttributeDescription attributeDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorWallVertex, position),
		},
		{
			.location = 1,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, position),
		},
		{
			.location = 2,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, scale),
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, axis),
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, centerOffset),
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorWallInstanceData, rotationQuat),
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
		.rasterizationState = &SHADOW_MAP_RASTERIZER,
		.multisampleState = &MULTISAMPLING_DISABLED,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &SHADOW_MAP_COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = shadowMapPipelineLayoutCreationInfo,
	};
	VulkanTestReturnResult(lunaCreateGraphicsPipelineWithVkRenderPass(device,
																	  &pipelineInfo,
																	  shadowMapRenderPass,
																	  0,
																	  &pipelines.shadowMaps.wallActors),
						   "Failed to create wall actor shadow map graphics pipeline!");

	return VK_SUCCESS;
}

bool CreateGraphicsPipelines()
{
	multisampling.rasterizationSamples = msaaSamples;
	pipelineLayoutCreationInfo.descriptorSetLayouts = &descriptorSetLayout;
	shadowMapPushConstantRange.dataPointer = &lightIndex;
	shadowMapPipelineLayoutCreationInfo.descriptorSetLayouts = &descriptorSetLayout;

	VulkanTest(CreateShaderModule(SHADER("model_shaded_f"), SHADER_TYPE_FRAG, &modelShadedFragShaderModule),
			   "Failed to load shaded model fragment shader!");
	VulkanTest(CreateShaderModule(SHADER("model_unshaded_f"), SHADER_TYPE_FRAG, &modelUnshadedFragShaderModule),
			   "Failed to load unshaded model fragment shader!");

	return CreateUIPipeline() &&
		   CreateShadedMapPipeline() &&
		   CreateUnshadedMapPipeline() &&
		   CreateSkyPipeline() &&
		   CreateShadedModelPipeline() &&
		   CreateUnshadedModelPipeline() &&
		   CreateShadedActorModelPipeline() &&
		   CreateUnshadedActorModelPipeline() &&
		   CreateActorWallPipelines() &&
		   CreateDebugDrawPipeline();
}

VkResult CreateShadowMapGraphicsPipelines()
{
	assert(shadowMapRenderPass != VK_NULL_HANDLE);

	VulkanTestReturnResult(CreateMapShadowMapPipeline(), "Failed to create map shadow maps pipeline!");
	VulkanTestReturnResult(CreateModelActorShadowMapPipeline(), "Failed to create model actor shadow maps pipeline!");
	VulkanTestReturnResult(CreateWallActorShadowMapPipeline(), "Failed to create wall actor shadow maps pipeline!");

	return VK_SUCCESS;
}
