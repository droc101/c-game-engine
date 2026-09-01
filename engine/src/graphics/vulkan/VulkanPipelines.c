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
	.depthBiasConstantFactor = -3.5f,
	.depthBiasSlopeFactor = -6.5f,
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
	.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
	.maxDepthBounds = 1,
};

static const VkPipelineDepthStencilStateCreateInfo DEPTH_STENCIL_EQUAL_STATE = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	.depthTestEnable = VK_TRUE,
	.depthCompareOp = VK_COMPARE_OP_EQUAL,
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

static LunaDescriptorSetLayout descriptorSetLayouts[3];
static const LunaPipelineLayoutCreationInfo PIPELINE_LAYOUT_CREATION_INFO = {
	.descriptorSetLayoutCount = 3,
	.descriptorSetLayouts = descriptorSetLayouts,
};

static LunaPushConstantsRange shadowMapPushConstantRange = {
	.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
	.size = sizeof(ShadowMapPushConstants),
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
static LunaShaderModule shadowMapsFragShaderModule = LUNA_NULL_HANDLE;

static const VkSpecializationMapEntry SPECIALIZATION_MAP_ENTRY = {
	.size = sizeof(uint32_t),
};
static const VkSpecializationInfo SPECIALIZATION_INFO = {
	.mapEntryCount = 1,
	.pMapEntries = &SPECIALIZATION_MAP_ENTRY,
	.dataSize = sizeof(uint32_t),
	.pData = &lightCount,
};
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
		.layoutCreationInfo = PIPELINE_LAYOUT_CREATION_INFO,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, "Main Pass"),
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
			.specializationInfo = &SPECIALIZATION_INFO,
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
		.depthStencilState = &DEPTH_STENCIL_EQUAL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = PIPELINE_LAYOUT_CREATION_INFO,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, "Main Pass"),
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
		.depthStencilState = &DEPTH_STENCIL_EQUAL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = PIPELINE_LAYOUT_CREATION_INFO,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, "Main Pass"),
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
		.descriptorSetLayoutCount = PIPELINE_LAYOUT_CREATION_INFO.descriptorSetLayoutCount,
		.descriptorSetLayouts = PIPELINE_LAYOUT_CREATION_INFO.descriptorSetLayouts,
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
										  lunaGetRenderPassSubpassByName(renderPass, "Main Pass"),
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
			.specializationInfo = &SPECIALIZATION_INFO,
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
		.layoutCreationInfo = PIPELINE_LAYOUT_CREATION_INFO,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, "Main Pass"),
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
		.layoutCreationInfo = PIPELINE_LAYOUT_CREATION_INFO,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, "Main Pass"),
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
			.specializationInfo = &SPECIALIZATION_INFO,
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
			.stride = sizeof(uint32_t),
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
			.format = VK_FORMAT_R32_UINT,
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
		.depthStencilState = &DEPTH_STENCIL_EQUAL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = PIPELINE_LAYOUT_CREATION_INFO,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, "Main Pass"),
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
			.stride = sizeof(uint32_t),
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
			.format = VK_FORMAT_R32_UINT,
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
		.depthStencilState = &DEPTH_STENCIL_EQUAL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = PIPELINE_LAYOUT_CREATION_INFO,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &pipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, "Main Pass"),
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
			.specializationInfo = &SPECIALIZATION_INFO,
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
			.stride = sizeof(uint32_t),
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
			.format = VK_FORMAT_R32_UINT,
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
		.depthStencilState = &DEPTH_STENCIL_EQUAL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = PIPELINE_LAYOUT_CREATION_INFO,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &shadedPipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, "Main Pass"),
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
		.depthStencilState = &DEPTH_STENCIL_EQUAL_STATE,
		.colorBlendState = &COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = PIPELINE_LAYOUT_CREATION_INFO,
	};
	VulkanTest(lunaCreateGraphicsPipeline(device,
										  &unshadedPipelineInfo,
										  lunaGetRenderPassSubpassByName(renderPass, "Main Pass"),
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
		.layoutCreationInfo = PIPELINE_LAYOUT_CREATION_INFO,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, "Main Pass"),
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
		.layoutCreationInfo = PIPELINE_LAYOUT_CREATION_INFO,
		.subpass = lunaGetRenderPassSubpassByName(renderPass, "Main Pass"),
	};
	VulkanTest(lunaCreateGraphicsPipeline(device, &trianglesPipelineInfo, &pipelines.debugDrawTriangles),
			   "Failed to create graphics pipeline for Jolt debug renderer triangles!");
#endif

	return true;
}

static inline VkResult CreateOpaqueMapDepthPipelines(const bool shadowMaps)
{
	if (shadowMaps && pipelines.shadowMaps.opaqueMap != LUNA_NULL_HANDLE)
	{
		lunaDestroyGraphicsPipeline(device, pipelines.shadowMaps.opaqueMap);
	}
	if (pipelines.depthPrepass.opaqueMap != LUNA_NULL_HANDLE)
	{
		lunaDestroyGraphicsPipeline(device, pipelines.depthPrepass.opaqueMap);
	}

	LunaShaderModule shaderModule = LUNA_NULL_HANDLE;
	VulkanTestReturnResult(CreateShaderModule(SHADER("map_opaque_shadow_maps_v"), SHADER_TYPE_VERT, &shaderModule),
						   "Failed to load opaque map shadow maps vertex shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = shaderModule,
			.specializationInfo = &SPECIALIZATION_INFO,
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

	LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = shadowMapPipelineLayoutCreationInfo,
	};
	pipelineInfo.rasterizationState = &RASTERIZER;
	pipelineInfo.multisampleState = &multisampling;
	VulkanTestReturnResult(lunaCreateGraphicsPipeline(device,
													  &pipelineInfo,
													  lunaGetRenderPassSubpassByName(renderPass, "Depth Prepass"),
													  &pipelines.depthPrepass.opaqueMap),
						   "Failed to create opaque map depth prepass graphics pipeline!");

	if (shadowMaps)
	{
		pipelineInfo.rasterizationState = &SHADOW_MAP_RASTERIZER;
		pipelineInfo.multisampleState = &MULTISAMPLING_DISABLED;
		VulkanTestReturnResult(lunaCreateGraphicsPipelineWithVkRenderPass(device,
																		  &pipelineInfo,
																		  shadowMapRenderPass,
																		  0,
																		  &pipelines.shadowMaps.opaqueMap),
							   "Failed to create opaque map shadow map graphics pipeline!");
	}

	return VK_SUCCESS;
}

static inline VkResult CreateMapShadowMapPipeline(const bool shadowMaps)
{
	if (shadowMaps)
	{
		if (pipelines.shadowMaps.map != LUNA_NULL_HANDLE)
		{
			lunaDestroyGraphicsPipeline(device, pipelines.shadowMaps.map);
		}
		if (pipelines.directionalLightShadowMaps.mapFrontFaces != LUNA_NULL_HANDLE)
		{
			lunaDestroyGraphicsPipeline(device, pipelines.directionalLightShadowMaps.mapFrontFaces);
		}
		if (pipelines.directionalLightShadowMaps.mapBackFaces != LUNA_NULL_HANDLE)
		{
			lunaDestroyGraphicsPipeline(device, pipelines.directionalLightShadowMaps.mapBackFaces);
		}
	}
	if (pipelines.depthPrepass.map != LUNA_NULL_HANDLE)
	{
		lunaDestroyGraphicsPipeline(device, pipelines.depthPrepass.map);
	}

	LunaShaderModule shaderModule = LUNA_NULL_HANDLE;
	VulkanTestReturnResult(CreateShaderModule(SHADER("map_shadow_maps_v"), SHADER_TYPE_VERT, &shaderModule),
						   "Failed to load map shadow maps vertex shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = shaderModule,
			.specializationInfo = &SPECIALIZATION_INFO,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = shadowMapsFragShaderModule,
			.specializationInfo = &SPECIALIZATION_INFO,
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
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(MapVertex, normal),
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

	LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &SHADOW_MAP_COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = shadowMapPipelineLayoutCreationInfo,
	};
	pipelineInfo.rasterizationState = &RASTERIZER;
	pipelineInfo.multisampleState = &multisampling;
	VulkanTestReturnResult(lunaCreateGraphicsPipeline(device,
													  &pipelineInfo,
													  lunaGetRenderPassSubpassByName(renderPass, "Depth Prepass"),
													  &pipelines.depthPrepass.map),
						   "Failed to create map depth prepass graphics pipeline!");

	if (shadowMaps)
	{
		pipelineInfo.rasterizationState = &SHADOW_MAP_RASTERIZER;
		pipelineInfo.multisampleState = &MULTISAMPLING_DISABLED;
		VulkanTestReturnResult(lunaCreateGraphicsPipelineWithVkRenderPass(device,
																		  &pipelineInfo,
																		  shadowMapRenderPass,
																		  0,
																		  &pipelines.shadowMaps.map),
							   "Failed to create map shadow map graphics pipeline!");
		VulkanTestReturnResult(lunaCreateGraphicsPipelineWithVkRenderPass(device,
																		  &pipelineInfo,
																		  shadowMapRenderPass,
																		  0,
																		  &pipelines.directionalLightShadowMaps
																				   .mapFrontFaces),
							   "Failed to create map directional light front faces shadow map graphics pipeline!");

		static const VkPipelineRasterizationStateCreateInfo BACK_FACES_RASTERIZER = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_FRONT_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_TRUE,
			.depthBiasConstantFactor = 1.0f,
			.depthBiasSlopeFactor = 2.5f,
			.lineWidth = 1,
		};
		pipelineInfo.rasterizationState = &BACK_FACES_RASTERIZER;
		VulkanTestReturnResult(lunaCreateGraphicsPipelineWithVkRenderPass(device,
																		  &pipelineInfo,
																		  shadowMapRenderPass,
																		  0,
																		  &pipelines.directionalLightShadowMaps
																				   .mapBackFaces),
							   "Failed to create map directional light back faces shadow map graphics pipeline!");
	}

	return VK_SUCCESS;
}

static inline VkResult CreateModelActorDepthPipelines(const bool shadowMaps)
{
	if (shadowMaps)
	{
		if (pipelines.shadowMaps.modelActors != LUNA_NULL_HANDLE)
		{
			lunaDestroyGraphicsPipeline(device, pipelines.shadowMaps.modelActors);
		}
		if (pipelines.directionalLightShadowMaps.modelActors != LUNA_NULL_HANDLE)
		{
			lunaDestroyGraphicsPipeline(device, pipelines.directionalLightShadowMaps.modelActors);
		}
	}
	if (pipelines.depthPrepass.modelActors != LUNA_NULL_HANDLE)
	{
		lunaDestroyGraphicsPipeline(device, pipelines.depthPrepass.modelActors);
	}

	LunaShaderModule shaderModule = LUNA_NULL_HANDLE;
	VulkanTestReturnResult(CreateShaderModule(SHADER("actor_model_shadow_maps_v"), SHADER_TYPE_VERT, &shaderModule),
						   "Failed to load model actor shadow maps vertex shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = shaderModule,
			.specializationInfo = &SPECIALIZATION_INFO,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = shadowMapsFragShaderModule,
			.specializationInfo = &SPECIALIZATION_INFO,
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
			.stride = sizeof(uint32_t),
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
			.format = VK_FORMAT_R32_UINT,
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(attributeDescriptions) / sizeof(*attributeDescriptions),
		.pVertexAttributeDescriptions = attributeDescriptions,
	};

	LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &SHADOW_MAP_COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = shadowMapPipelineLayoutCreationInfo,
	};
	pipelineInfo.rasterizationState = &RASTERIZER;
	pipelineInfo.multisampleState = &multisampling;
	VulkanTestReturnResult(lunaCreateGraphicsPipeline(device,
													  &pipelineInfo,
													  lunaGetRenderPassSubpassByName(renderPass, "Depth Prepass"),
													  &pipelines.depthPrepass.modelActors),
						   "Failed to create model actor depth prepass graphics pipeline!");

	if (shadowMaps)
	{
		pipelineInfo.rasterizationState = &SHADOW_MAP_RASTERIZER;
		pipelineInfo.multisampleState = &MULTISAMPLING_DISABLED;
		VulkanTestReturnResult(lunaCreateGraphicsPipelineWithVkRenderPass(device,
																		  &pipelineInfo,
																		  shadowMapRenderPass,
																		  0,
																		  &pipelines.shadowMaps.modelActors),
							   "Failed to create model actor shadow map graphics pipeline!");
		VulkanTestReturnResult(lunaCreateGraphicsPipelineWithVkRenderPass(device,
																		  &pipelineInfo,
																		  shadowMapRenderPass,
																		  0,
																		  &pipelines.directionalLightShadowMaps
																				   .modelActors),
							   "Failed to create model actor directional light shadow map graphics pipeline!");
	}

	return VK_SUCCESS;
}

static inline VkResult CreateWallActorDepthPipelines(const bool shadowMaps)
{
	if (shadowMaps)
	{
		if (pipelines.shadowMaps.wallActors != LUNA_NULL_HANDLE)
		{
			lunaDestroyGraphicsPipeline(device, pipelines.shadowMaps.wallActors);
		}
		if (pipelines.directionalLightShadowMaps.wallActors != LUNA_NULL_HANDLE)
		{
			lunaDestroyGraphicsPipeline(device, pipelines.directionalLightShadowMaps.wallActors);
		}
	}
	if (pipelines.depthPrepass.wallActors != LUNA_NULL_HANDLE)
	{
		lunaDestroyGraphicsPipeline(device, pipelines.depthPrepass.wallActors);
	}

	LunaShaderModule shaderModule = LUNA_NULL_HANDLE;
	VulkanTestReturnResult(CreateShaderModule(SHADER("actor_wall_shadow_maps_v"), SHADER_TYPE_VERT, &shaderModule),
						   "Failed to load wall actor shadow maps vertex shader!");

	const LunaPipelineShaderStageCreationInfo shaderStages[] = {
		{
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = shaderModule,
			.specializationInfo = &SPECIALIZATION_INFO,
		},
		{
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = shadowMapsFragShaderModule,
			.specializationInfo = &SPECIALIZATION_INFO,
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
			.stride = sizeof(uint32_t),
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
			.format = VK_FORMAT_R32_UINT,
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = sizeof(bindingDescriptions) / sizeof(*bindingDescriptions),
		.pVertexBindingDescriptions = bindingDescriptions,
		.vertexAttributeDescriptionCount = sizeof(attributeDescriptions) / sizeof(*attributeDescriptions),
		.pVertexAttributeDescriptions = attributeDescriptions,
	};

	LunaGraphicsPipelineCreationInfo pipelineInfo = {
		.shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &INPUT_ASSEMBLY,
		.viewportState = &VIEWPORT_STATE,
		.depthStencilState = &DEPTH_STENCIL_STATE,
		.colorBlendState = &SHADOW_MAP_COLOR_BLENDING,
		.dynamicState = &DYNAMIC_STATE,
		.layoutCreationInfo = shadowMapPipelineLayoutCreationInfo,
	};
	pipelineInfo.rasterizationState = &RASTERIZER;
	pipelineInfo.multisampleState = &multisampling;
	VulkanTestReturnResult(lunaCreateGraphicsPipeline(device,
													  &pipelineInfo,
													  lunaGetRenderPassSubpassByName(renderPass, "Depth Prepass"),
													  &pipelines.depthPrepass.wallActors),
						   "Failed to create wall actor depth prepass graphics pipeline!");

	if (shadowMaps)
	{
		pipelineInfo.rasterizationState = &SHADOW_MAP_RASTERIZER;
		pipelineInfo.multisampleState = &MULTISAMPLING_DISABLED;
		VulkanTestReturnResult(lunaCreateGraphicsPipelineWithVkRenderPass(device,
																		  &pipelineInfo,
																		  shadowMapRenderPass,
																		  0,
																		  &pipelines.shadowMaps.wallActors),
							   "Failed to create wall actor shadow map graphics pipeline!");
		VulkanTestReturnResult(lunaCreateGraphicsPipelineWithVkRenderPass(device,
																		  &pipelineInfo,
																		  shadowMapRenderPass,
																		  0,
																		  &pipelines.directionalLightShadowMaps
																				   .wallActors),
							   "Failed to create wall actor directional light shadow map graphics pipeline!");
	}

	return VK_SUCCESS;
}

bool CreateCullingDataClearPipeline()
{
	const LunaPushConstantsRange pushConstantsRange = {
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.size = sizeof(actorModelsDrawInfoCount),
		.dataPointer = &actorModelsDrawInfoCount,
	};
	const LunaPipelineLayoutCreationInfo layoutCreationInfo = {
		.descriptorSetLayoutCount = 1,
		.descriptorSetLayouts = &descriptorSets.culling.layout,
		.pushConstantRangeCount = 1,
		.pushConstantsRanges = &pushConstantsRange,
	};
	LunaShaderModule shaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("clear_culling_data_c"), SHADER_TYPE_COMP, &shaderModule),
			   "Failed to load culling data clear shader!");
	const LunaPipelineShaderStageCreationInfo shaderStageCreationInfo = {
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.module = shaderModule,
	};
	const LunaComputePipelineCreationInfo creationInfo = {
		.shaderStageCreationInfo = shaderStageCreationInfo,
		.layoutCreationInfo = layoutCreationInfo,
	};
	VulkanTest(lunaCreateComputePipeline(device, &creationInfo, &pipelines.clearCullingData),
			   "Failed to create culling pipeline!");

	return true;
}

bool CreateCullingPipeline()
{
	const LunaDescriptorSetLayout layouts[] = {
		descriptorSets.common.layout,
		descriptorSets.culling.layout,
	};
	const LunaPipelineLayoutCreationInfo layoutCreationInfo = {
		.descriptorSetLayoutCount = 2,
		.descriptorSetLayouts = layouts,
	};
	LunaShaderModule shaderModule = LUNA_NULL_HANDLE;
	VulkanTest(CreateShaderModule(SHADER("culling_c"), SHADER_TYPE_COMP, &shaderModule),
			   "Failed to load culling shader!");
	const LunaPipelineShaderStageCreationInfo shaderStageCreationInfo = {
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.module = shaderModule,
	};
	const LunaComputePipelineCreationInfo creationInfo = {
		.shaderStageCreationInfo = shaderStageCreationInfo,
		.layoutCreationInfo = layoutCreationInfo,
	};
	VulkanTest(lunaCreateComputePipeline(device, &creationInfo, &pipelines.culling),
			   "Failed to create culling pipeline!");

	return true;
}

bool CreateGraphicsPipelines()
{
	multisampling.rasterizationSamples = msaaSamples;
	descriptorSetLayouts[0] = descriptorSets.common.layout;
	descriptorSetLayouts[1] = descriptorSets.spotLightShadowMaps.layout;
	descriptorSetLayouts[2] = descriptorSets.pointLightShadowMaps.layout;

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

VkResult CreateDepthGraphicsPipelines(void)
{
	shadowMapPushConstantRange.dataPointer = &shadowMapPushConstants;
	shadowMapPipelineLayoutCreationInfo.descriptorSetLayouts = &descriptorSets.common.layout;

	VulkanTestReturnResult(CreateShaderModule(SHADER("shadow_maps_f"), SHADER_TYPE_FRAG, &shadowMapsFragShaderModule),
						   "Failed to load spot light shadow maps fragment shader!");

	VulkanTestReturnResult(CreateOpaqueMapDepthPipelines(shadowMapRenderPass != VK_NULL_HANDLE),
						   "Failed to create map shadow maps pipeline!");
	VulkanTestReturnResult(CreateMapShadowMapPipeline(shadowMapRenderPass != VK_NULL_HANDLE),
						   "Failed to create map shadow maps pipeline!");
	VulkanTestReturnResult(CreateModelActorDepthPipelines(shadowMapRenderPass != VK_NULL_HANDLE),
						   "Failed to create model actor shadow maps pipeline!");
	VulkanTestReturnResult(CreateWallActorDepthPipelines(shadowMapRenderPass != VK_NULL_HANDLE),
						   "Failed to create wall actor shadow maps pipeline!");

	return VK_SUCCESS;
}

bool UpdateLightCount()
{
	VulkanTest(CreateDepthGraphicsPipelines(), "Failed to create shadow map graphics pipelines");

	lunaDestroyGraphicsPipeline(device, pipelines.shadedMap);
	lunaDestroyGraphicsPipeline(device, pipelines.shadedModel);
	lunaDestroyGraphicsPipeline(device, pipelines.shadedActorModel);
	lunaDestroyGraphicsPipeline(device, pipelines.shadedActorWall);
	lunaDestroyGraphicsPipeline(device, pipelines.unshadedActorWall);

	return CreateShadedMapPipeline() &&
		   CreateShadedModelPipeline() &&
		   CreateShadedActorModelPipeline() &&
		   CreateActorWallPipelines();
}
