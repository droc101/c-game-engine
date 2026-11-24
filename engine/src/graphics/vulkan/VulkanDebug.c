//
// Created by NBT22 on 9/1/26.
//

#include <assert.h>
#include <cglm/cglm.h>
#include <cglm/clipspace/ortho_lh_zo.h>
#include <cglm/clipspace/persp_lh_zo.h>
#include <cglm/clipspace/view_lh_zo.h>
#include <cglm/types.h>
#include <engine/assets/ModelLoader.h>
#include <engine/debug/DPrint.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/graphics/vulkan/VulkanDebug.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/helpers/Macros.h>
#include <engine/structs/Color.h>
#include <joltc/Math/Vector3.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaTypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>

void VK_DPrintDevice()
{
	const char *gpuType = "Unknown";
	switch (physicalDeviceProperties.deviceType)
	{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			gpuType = "Integrated";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			gpuType = "Discrete";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			gpuType = "Virtual";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			gpuType = "CPU";
			break;
		default:
			break;
	}

	DPrintF("GPU: %s (%s)", COLOR_WHITE, physicalDeviceProperties.deviceName, gpuType);
	DPrintF("Vulkan: %u.%u.%u",
			COLOR_WHITE,
			VK_API_VERSION_MAJOR(physicalDeviceProperties.apiVersion),
			VK_API_VERSION_MINOR(physicalDeviceProperties.apiVersion),
			VK_API_VERSION_PATCH(physicalDeviceProperties.apiVersion));
}

void VK_DPrintSceneStatistics()
{
	DPrintF("Vulkan scene statistics:", COLOR_WHITE);
	DPrintF("Map vertex/index count: %u/%u",
			COLOR_WHITE,
			lunaGetBufferSize(buffers.map.vertices) / sizeof(ModelVertex),
			lunaGetBufferSize(buffers.map.indices) / sizeof(uint32_t));
	DPrintF("Model vertex/index count: %u/%u",
			COLOR_WHITE,
			lunaGetBufferSize(buffers.modelVertices) / sizeof(ModelVertex),
			lunaGetBufferSize(buffers.modelIndices) / sizeof(uint32_t));
	DPrintF("Actor wall shaded/unshaded count: %u/%u",
			COLOR_WHITE,
			buffers.actorWalls.shadedInstanceCount,
			buffers.actorWalls.unshadedInstanceCount);
}

void VK_ToggleBakedLighting()
{
	specializationConstants.bakedLighting = !specializationConstants.bakedLighting;
	pendingTasks |= PENDING_TASK_RECREATE_GRAPHICS_PIPELINES;
}

void VK_SetDebugRendering(const DebugRendering value)
{
	specializationConstants.debugRendering = value;
	rendererQueuedActions |= QUEUED_ACTION_RELOAD_ALL_SHADERS;
}

void VK_BeginDebugLabel(const char *formatString, ...)
{
	if (debugUtils)
	{
		char message[1024];
		va_list args;
		va_start(args, formatString);
		vsnprintf(message, 1024, formatString, args);
		va_end(args);
		message[1023] = '\0';

		const VkDebugUtilsLabelEXT label = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
			.pLabelName = message,
		};
		vkCmdBeginDebugUtilsLabelEXT(vkCommandBuffer, &label);
	}
}

void VK_EndDebugLabel()
{
	if (debugUtils)
	{
		vkCmdEndDebugUtilsLabelEXT(vkCommandBuffer);
	}
}

CullingSpheresFilters cullingSpheresFilters = {
	.opaqueMapModels = CULLING_SPHERES_FILTER_NONE,
	.translucentMapModels = CULLING_SPHERES_FILTER_NONE,
	.actorModels = CULLING_SPHERES_FILTER_BOTH,
	.actorWalls = CULLING_SPHERES_FILTER_BOTH,
};

typedef struct CullingSpheresDrawInfo
{
	uint32_t filter;
	CullingSpheresFilter filterBit;
	VkDeviceAddress cullingInfo;
	bool isActorModelBuffer;
	Vector3 color;
} CullingSpheresDrawInfo;

void DrawCullingSpheres(const LunaGraphicsPipelineBindInfo *pipelineBindInfo)
{
	if (specializationConstants.debugRendering != DEBUG_RENDERING_CULLING_SPHERES ||
		maximumCulledInstanceCount == 0 ||
		frustumCullingDatas == NULL)
	{
		return;
	}

	VK_BeginDebugLabel("Draw culling spheres");

	const CullingSpheresDrawInfo drawInfos[] = {
		{
			.filter = cullingSpheresFilters.opaqueMapModels,
			.filterBit = CULLING_SPHERES_FILTER_SHADED,
			.cullingInfo = frustumCullingDatas->shadedOpaqueMapModelsCullingInfo,
			.color = {0.1f, 1.0f, 0.1f},
		},
		{
			.filter = cullingSpheresFilters.opaqueMapModels,
			.filterBit = CULLING_SPHERES_FILTER_UNSHADED,
			.cullingInfo = frustumCullingDatas->unshadedOpaqueMapModelsCullingInfo,
			.color = {1.0f, 0.4f, 0.1f},
		},
		{
			.filter = cullingSpheresFilters.translucentMapModels,
			.filterBit = CULLING_SPHERES_FILTER_SHADED,
			.cullingInfo = frustumCullingDatas->shadedMapModelsCullingInfo,
			.color = {0.1f, 0.5f, 0.1f},
		},
		{
			.filter = cullingSpheresFilters.translucentMapModels,
			.filterBit = CULLING_SPHERES_FILTER_UNSHADED,
			.cullingInfo = frustumCullingDatas->unshadedMapModelsCullingInfo,
			.color = {1.0f, 0.8f, 0.1f},
		},
		{
			.filter = cullingSpheresFilters.actorModels,
			.filterBit = CULLING_SPHERES_FILTER_SHADED,
			.cullingInfo = frustumCullingDatas->shadedActorModelsCullingInfo,
			.isActorModelBuffer = true,
			.color = {1.0f, 0.1f, 1.0f},
		},
		{
			.filter = cullingSpheresFilters.actorModels,
			.filterBit = CULLING_SPHERES_FILTER_UNSHADED,
			.cullingInfo = frustumCullingDatas->unshadedActorModelsCullingInfo,
			.isActorModelBuffer = true,
			.color = {0.1f, 1.0f, 1.0f},
		},
		{
			.filter = cullingSpheresFilters.actorWalls,
			.filterBit = CULLING_SPHERES_FILTER_SHADED,
			.cullingInfo = frustumCullingDatas->shadedActorWallsCullingInfo,
			.color = {0.1f, 0.5f, 1.0f},
		},
		{
			.filter = cullingSpheresFilters.actorWalls,
			.filterBit = CULLING_SPHERES_FILTER_UNSHADED,
			.cullingInfo = frustumCullingDatas->unshadedActorWallsCullingInfo,
			.color = {0.6f, 0.2f, 1.0f},
		},
	};

	for (uint32_t i = 0; i < ArrayLength(drawInfos); i++)
	{
		const CullingSpheresDrawInfo *drawInfo = &drawInfos[i];
		if ((drawInfo->filter & drawInfo->filterBit) == 0 || drawInfo->cullingInfo == 0)
		{
			continue;
		}

		cullingSpheresPushConstants.cullingInfo = drawInfo->cullingInfo;
		cullingSpheresPushConstants.color = drawInfo->color;
		cullingSpheresPushConstants.isActorModelBuffer = drawInfo->isActorModelBuffer;
		cullingSpheresPushConstants.keptPixelIndexOffset = i;
		VulkanTest(lunaPushConstants(device, commandBuffer, pipelines.cullingSpheres),
				   "Failed to push constants for culling spheres pipeline!");
		const LunaDrawIndexedInfo lunaDrawInfo = {
			.pipeline = pipelines.cullingSpheres,
			.pipelineBindInfo = pipelineBindInfo,
			.indexCount = lunaGetBufferSize(buffers.cullingSpheres.indices) / sizeof(uint32_t),
			.instanceCount = maximumCulledInstanceCount,
		};
		VulkanTest(lunaDrawBufferIndexed(device,
										 commandBuffer,
										 buffers.cullingSpheres.vertices,
										 buffers.cullingSpheres.indices,
										 VK_INDEX_TYPE_UINT32,
										 &lunaDrawInfo),
				   "Failed to draw culling spheres!");
	}

	VK_EndDebugLabel();
}
