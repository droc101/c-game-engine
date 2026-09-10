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
#include <engine/graphics/vulkan/VulkanDebug.h>
#include <engine/graphics/vulkan/VulkanHelpers.h>
#include <engine/structs/Color.h>
#include <luna/lunaBuffer.h>
#include <stddef.h>
#include <stdint.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>

#include "engine/graphics/RenderingHelpers.h"

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
	DPrintF("Actor model vertex/index count: %u/%u",
			COLOR_WHITE,
			lunaGetBufferSize(buffers.actorModels.vertices) / sizeof(ModelVertex),
			lunaGetBufferSize(buffers.actorModels.indices) / sizeof(uint32_t));
	DPrintF("Actor wall shaded/unshaded count: %u/%u",
			COLOR_WHITE,
			buffers.actorWalls.shadedInstanceCount,
			buffers.actorWalls.unshadedInstanceCount);
}

bool VK_ToggleIndirectLight()
{
	pendingTasks |= PENDING_TASK_RELOAD_LIGHTING_SHADERS;
}
