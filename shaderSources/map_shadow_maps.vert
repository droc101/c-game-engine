#version 460

#include "shared.inc.glsl"

layout(push_constant) uniform PushConstants {
    uint lightIndex;
    uint faceIndex;
} pushConstants;

layout(scalar, set = 0, binding = 5) readonly restrict buffer LightsData {
	uint lightCount;
    Light lights[];
} lightsData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in uint inTextureIndex;

layout(location = 0) out vec4 outUvAlphaDistance;
layout(location = 1) flat out uint outTextureIndex;

void main() {
    outTextureIndex = inTextureIndex;
	if (lightsData.lights[pushConstants.lightIndex].type == LIGHT_TYPE_POINT) {
        const vec3 lightToWorld = inPosition - lightsData.lights[pushConstants.lightIndex].position;
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * (transforms[pushConstants.faceIndex] * vec4(-lightToWorld.xy, lightToWorld.z, 1));
        outUvAlphaDistance = vec4(inUV, 1, length(lightToWorld));
	} else {
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * vec4(inPosition - lightsData.lights[pushConstants.lightIndex].position, 1);
        outUvAlphaDistance = vec4(inUV, 1, 0);
	}
}
