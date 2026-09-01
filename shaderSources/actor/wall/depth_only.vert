#version 460

#include "../../include/shadow_maps_shared.inc.glsl"

layout(binding = 7, scalar) readonly restrict buffer InstanceData {
    ActorWallInstanceData instanceDatas[];
} instanceDatas;

layout(location = 0) in vec2 inVertexPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in uint inInstanceIndex;

layout(location = 0, component = 0) out vec2 outUv;
layout(location = 0, component = 3) out float outAlpha;
layout(location = 1) flat out uint outTextureIndex;

vec3 rotateVec3ByQuat(vec3 point, vec4 quat){ 
  return point + 2.0 * cross(quat.xyz, cross(quat.xyz, point) + quat.w * point);
}

vec3 getVec3FromVec2(const vec2 vec, const vec2 axis) {
    return vec3(vec.x * axis.x, vec.y, vec.x * axis.y);
}

void main() {
    const ActorWallInstanceData instanceData = instanceDatas.instanceDatas[inInstanceIndex];
    outUv = inUV * instanceData.uvScale * (instanceData.scale / vec2(16.0)) + instanceData.uvOffset;
	outAlpha = instanceData.modColor.a;
	outTextureIndex = instanceData.textureIndex;
	const vec3 position = rotateVec3ByQuat(getVec3FromVec2(inVertexPosition * instanceData.scale + instanceData.centerOffset, instanceData.axis), instanceData.rotationQuat) + instanceData.position;
	if (pushConstants.lightType == LIGHT_TYPE_POINT) {
        const vec3 lightToWorld = position - lightsData.lights[pushConstants.lightIndex].transform.position;
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * (pointLightViewMatrices[pushConstants.faceIndex] * vec4(lightToWorld, 1));
	} else if (pushConstants.lightType == LIGHT_TYPE_SPOT) {
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * vec4(position, 1);
	} else if (pushConstants.lightType == LIGHT_TYPE_DIRECTIONAL) {
	    gl_Position = lightsData.cascadeMatrices[pushConstants.cascadeIndex] * vec4(position, 1);
		gl_Position.z = min(gl_Position.z, 1);
	} else {
		gl_Position = camera.transformMatrix * vec4(position, 1);
	}
}
