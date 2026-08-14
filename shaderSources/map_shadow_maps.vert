#version 460

#include "shadow_maps_shared.inc.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in uint inTextureIndex;

layout(location = 0, component = 0) out vec2 outUv;
layout(location = 0, component = 3) out float outAlpha;
layout(location = 1) out vec3 outPosition;
layout(location = 2) flat out uint outTextureIndex;

void main() {
	outUv = inUV;
	outAlpha = 1;
	outPosition = inPosition;
    outTextureIndex = inTextureIndex;
	if (lightsData.lights[pushConstants.lightIndex].type == LIGHT_TYPE_POINT) {
        const vec3 lightToWorld = inPosition - lightsData.lights[pushConstants.lightIndex].transform.position;
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * (transforms[pushConstants.faceIndex] * vec4(-lightToWorld.xy, lightToWorld.z, 1));
	} else if (lightsData.lights[pushConstants.lightIndex].type == LIGHT_TYPE_SPOT) {
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * vec4(inPosition, 1);
	} else {
	    gl_Position = lightsData.cascadeMatrices[pushConstants.cascadeIndex] * vec4(inPosition, 1);
	}
}
