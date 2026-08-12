// Include only. This file will not compile as a standalone module.

#include "shared.inc.glsl"

layout(push_constant) uniform PushConstants {
    uint lightIndex;
    uint faceIndex;
    uint cascadeIndex;
} pushConstants;

layout(scalar, set = 0, binding = 5) readonly restrict buffer LightsData {
	uint lightCount;
	mat4 cascades[4];
    Light lights[];
} lightsData;