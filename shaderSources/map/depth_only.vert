#version 460

#include "../include/shadow_maps_shared.inc.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in uint inTextureIndex;

layout(location = 0, component = 0) out vec2 outUv;
layout(location = 0, component = 3) out float outAlpha;
layout(location = 1) flat out uint outTextureIndex;

void main() {
	outUv = inUV;
	outAlpha = 1;
    outTextureIndex = inTextureIndex;
	if (pushConstants.lightType == LIGHT_TYPE_POINT) {
        const vec3 lightToWorld = inPosition - lightsData.lights[pushConstants.lightIndex].transform.position;
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * (pointLightViewMatrices[pushConstants.faceIndex] * vec4(lightToWorld, 1));
	} else if (pushConstants.lightType == LIGHT_TYPE_SPOT) {
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * vec4(inPosition, 1);
	} else if (pushConstants.lightType == LIGHT_TYPE_DIRECTIONAL) {
	    gl_Position = lightsData.cascadeMatrices[pushConstants.cascadeIndex] * vec4(inPosition, 1);
		gl_Position.z = min(gl_Position.z, 1);
	} else {
		gl_Position = camera.transformMatrix * vec4(inPosition, 1);
	}
}
