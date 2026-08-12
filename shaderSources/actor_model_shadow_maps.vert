#version 460

#include "shadow_maps_shared.inc.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inColor;
layout(location = 4) in mat4 inTransformMatrix;
layout(location = 8) in vec4 inModColor;
layout(location = 9) in vec4 inMaterialColor;
layout(location = 10) in uint inTextureIndex;

layout(location = 0) out vec3 outUvAlpha;
layout(location = 1) out vec3 outPosition;
layout(location = 2) flat out uint outTextureIndex;

void main() {
    outUvAlpha = vec3(inUV, inColor.a * inMaterialColor.a * inModColor.a);
	outPosition = (inTransformMatrix * vec4(inPosition, 1)).xyz;
    outTextureIndex = inTextureIndex;
	if (lightsData.lights[pushConstants.lightIndex].type == LIGHT_TYPE_POINT) {
        const vec3 lightToWorld = outPosition - lightsData.lights[pushConstants.lightIndex].position;
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * (transforms[pushConstants.faceIndex] * vec4(-lightToWorld.xy, lightToWorld.z, 1));
	} else {
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * inTransformMatrix * vec4(inPosition, 1);
	}
}
