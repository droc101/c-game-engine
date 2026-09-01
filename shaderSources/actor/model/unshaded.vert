#version 460

#include "../../include/shared.inc.glsl"

layout(binding = 7, scalar) readonly restrict buffer InstanceData {
    ActorModelInstanceData instanceDatas[];
} instanceDatas[2];

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;
layout(location = 3) in uint inInstanceIndex;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) flat out uint outTextureIndex;

void main() {
    const ActorModelInstanceData instanceData = instanceDatas[1].instanceDatas[inInstanceIndex];
    outColor = inColor * instanceData.materialColor * instanceData.modColor;
    outUV = inUV;
    outTextureIndex = instanceData.textureIndex;
    gl_Position = camera.transformMatrix * (instanceData.transformMatrix * vec4(inPosition, 1));
}
