// Include only. This file will not compile as a standalone module.

#include "shared.inc.glsl"

layout(push_constant) uniform PushConstants {
    uint lightIndex;
    uint faceIndex;
    uint cascadeIndex;
} pushConstants;

layout(scalar, set = 0, binding = 5) readonly restrict buffer LightsData {
	uint lightCount;
    float cascadeDepths[4];
	mat4 cascadeMatrices[4];
    Light lights[];
} lightsData;

const mat4 pointLightViewMatrices[6] = {
	mat4(
         0,  0,  1,  0,
         0, -1,  0,  0,
         1,  0,  0,  0,
         0,  0,  0,  1
	),
    mat4(
         0,  0, -1,  0,
         0, -1,  0,  0,
        -1,  0,  0,  0,
         0,  0,  0,  1
	),
	mat4(
         1,  0,  0,  0,
         0,  0,  1,  0,
         0, -1,  0,  0,
         0,  0,  0,  1
	),
	mat4(
         1,  0,  0,  0,
         0,  0, -1,  0,
         0,  1,  0,  0,
         0,  0,  0,  1
	),
	mat4(
         1,  0,  0,  0,
         0, -1,  0,  0,
         0,  0, -1,  0,
         0,  0,  0,  1
	),
	mat4(
        -1,  0,  0,  0,
         0, -1,  0,  0,
         0,  0,  1,  0,
         0,  0,  0,  1
	),
};