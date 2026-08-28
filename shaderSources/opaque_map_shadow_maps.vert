#version 460

#include "shadow_maps_shared.inc.glsl"

layout(location = 0) in vec3 inPosition;

void main() {
	if (lightsData.lights[pushConstants.lightIndex].type == LIGHT_TYPE_POINT) {
        const vec3 lightToWorld = inPosition - lightsData.lights[pushConstants.lightIndex].transform.position;
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * (pointLightViewMatrices[pushConstants.faceIndex] * vec4(lightToWorld, 1));
	} else if (lightsData.lights[pushConstants.lightIndex].type == LIGHT_TYPE_SPOT) {
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * vec4(inPosition, 1);
	} else {
	    gl_Position = lightsData.cascadeMatrices[pushConstants.cascadeIndex] * vec4(inPosition, 1);
		gl_Position.z = min(gl_Position.z, 1);
	}
}
