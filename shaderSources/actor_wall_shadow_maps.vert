#version 460

#include "shadow_maps_shared.inc.glsl"

layout(location = 0) in vec2 inVertexPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inActorPosition;
layout(location = 3) in vec2 inScale;
layout(location = 4) in vec2 inAxis;
layout(location = 5) in vec2 inCenterOffset;
layout(location = 6) in vec4 inRotationQuat;
layout(location = 7) in uint inTextureIndex;
layout(location = 8) in vec2 inUvScale;
layout(location = 9) in vec2 inUvOffset;
layout(location = 10) in vec4 inModColor;

layout(location = 0, component = 0) out vec2 outUv;
layout(location = 0, component = 3) out float outAlpha;
layout(location = 1) out vec3 outPosition;
layout(location = 2) flat out uint outTextureIndex;

vec3 rotateVec3ByQuat(vec3 point, vec4 quat){ 
  return point + 2.0 * cross(quat.xyz, cross(quat.xyz, point) + quat.w * point);
}

vec3 getVec3FromVec2(vec2 vec) {
  return vec3(vec.x * inAxis.x, vec.y, vec.x * inAxis.y);
}

void main() {
    outUv = inUV * inUvScale * (inScale / vec2(16.0)) + inUvOffset;
	outAlpha = inModColor.a;
	outPosition = rotateVec3ByQuat(getVec3FromVec2(inVertexPosition * inScale + inCenterOffset), inRotationQuat) + inActorPosition;
    outTextureIndex = inTextureIndex;
	if (lightsData.lights[pushConstants.lightIndex].type == LIGHT_TYPE_POINT) {
        const vec3 lightToWorld = outPosition - lightsData.lights[pushConstants.lightIndex].transform.position;
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * (pointLightViewMatrices[pushConstants.faceIndex] * vec4(-lightToWorld.xy, lightToWorld.z, 1));
	} else if (lightsData.lights[pushConstants.lightIndex].type == LIGHT_TYPE_SPOT) {
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * vec4(outPosition, 1);
	} else {
	    gl_Position = lightsData.cascadeMatrices[pushConstants.cascadeIndex] * vec4(outPosition, 1);
		gl_Position.z = min(gl_Position.z, 1);
	}
}
