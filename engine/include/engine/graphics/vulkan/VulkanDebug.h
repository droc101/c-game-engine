//
// Created by NBT22 on 9/1/26.
//

#ifndef GAME_VULKANDEBUG_H
#define GAME_VULKANDEBUG_H

#include <stdint.h>

typedef enum DebugRendering
{
	DEBUG_RENDERING_DISABLED,
	DEBUG_RENDERING_DISABLE_LIGHTING,
	DEBUG_RENDERING_UNTEXTURED,
	DEBUG_RENDERING_NORMALS,
	DEBUG_RENDERING_UVS,
	DEBUG_RENDERING_SHOW_CLUSTERS,
	DEBUG_RENDERING_SHOW_CLUSTER_LIGHT_COUNTS,
	DEBUG_RENDERING_ONLY_LIGHTING,
	DEBUG_RENDERING_NO_LIGHT_FALLOFF,
	DEBUG_RENDERING_DEPTH_ONLY,
	DEBUG_RENDERING_PRIMITIVES,
	DEBUG_RENDERING_DISABLE_DEPTH_PREPASS,
	DEBUG_RENDERING_CULLING_SPHERES,

	/// @private
	DEBUG_RENDERING_COUNT,
} DebugRendering;

typedef enum CullingSpheresFilter
{
	CULLING_SPHERES_FILTER_NONE,
	CULLING_SPHERES_FILTER_SHADED,
	CULLING_SPHERES_FILTER_UNSHADED,
	CULLING_SPHERES_FILTER_BOTH,
} CullingSpheresFilter;

typedef struct CullingSpheresFilters
{
	uint32_t opaqueMapModels;
	uint32_t translucentMapModels;
	uint32_t actorModels;
	uint32_t actorWalls;
} CullingSpheresFilters;

extern CullingSpheresFilters cullingSpheresFilters;

void VK_DPrintDevice();

void VK_DPrintSceneStatistics();

void VK_ToggleBakedLighting();

void VK_SetDebugRendering(DebugRendering value);

void VK_BeginDebugLabel(const char *formatString, ...);

void VK_EndDebugLabel();

#endif //GAME_VULKANDEBUG_H
