// Include only. This file will not compile as a standalone module.

#include "shared.inc.glsl"

layout(constant_id = 0) const uint LIGHT_COUNT = 1;

layout(push_constant) uniform PushConstants {
    uint lightType;
    uint lightIndex;
    uint faceIndex;
    uint cascadeIndex;
} pushConstants;

layout(set = 0, binding = 5, scalar) readonly restrict uniform LightsData {
    float cascadeDepths[4];
	mat4 cascadeMatrices[4];
    Light lights[LIGHT_COUNT];
} lightsData;

const mat4 pointLightViewMatrices[6] = {
	mat4(
         0,  0, -1,  0,
         0,  1,  0,  0,
         1,  0,  0,  0,
         0,  0,  0,  1
	),
    mat4(
         0,  0,  1,  0,
         0,  1,  0,  0,
        -1,  0,  0,  0,
         0,  0,  0,  1
	),
	mat4(
        -1,  0,  0,  0,
         0,  0, -1,  0,
         0, -1,  0,  0,
         0,  0,  0,  1
	),
	mat4(
        -1,  0,  0,  0,
         0,  0,  1,  0,
         0,  1,  0,  0,
         0,  0,  0,  1
	),
	mat4(
        -1,  0,  0,  0,
         0,  1,  0,  0,
         0,  0, -1,  0,
         0,  0,  0,  1
	),
	mat4(
         1,  0,  0,  0,
         0,  1,  0,  0,
         0,  0,  1,  0,
         0,  0,  0,  1
	),
};