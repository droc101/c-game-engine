#version 460

#include "shared.inc.glsl"

layout(binding = 7, scalar) readonly restrict buffer InstanceData {
    ActorWallInstanceData instanceDatas[];
} instanceDatas;

layout(location = 0) in vec2 inVertexPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in uint inInstanceIndex;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) flat out uint outTextureIndex;

vec3 rotateVec3ByQuat(vec3 point, vec4 quat){ 
    return point + 2.0 * cross(quat.xyz, cross(quat.xyz, point) + quat.w * point);
}

vec3 getVec3FromVec2(const vec2 vec, const vec2 axis) {
    return vec3(vec.x * axis.x, vec.y, vec.x * axis.y);
}

void main() {
    const ActorWallInstanceData instanceData = instanceDatas.instanceDatas[inInstanceIndex];
    outColor = instanceData.modColor;
    outUV = inUV * instanceData.uvScale * (instanceData.scale / vec2(16.0)) + instanceData.uvOffset;
    outTextureIndex = instanceData.textureIndex;
    gl_Position = camera.transformMatrix * vec4(rotateVec3ByQuat(getVec3FromVec2(inVertexPosition * instanceData.scale + instanceData.centerOffset, instanceData.axis), instanceData.rotationQuat) + instanceData.position, 1);
}
