#version 460

#include "shared.inc.glsl"

layout(binding = 7, scalar) readonly restrict buffer InstanceData {
    ActorModelInstanceData instanceDatas[];
} instanceDatas[2];

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in uint inInstanceIndex;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out float outDistance;
layout(location = 5) flat out uint outTextureIndex;

void main() {
    const ActorModelInstanceData instanceData = instanceDatas[1].instanceDatas[inInstanceIndex];
	outPosition = instanceData.transformMatrix * vec4(inPosition, 1);
    outColor = inColor * instanceData.materialColor * instanceData.modColor;
    outUV = inUV;
    outNormal = (instanceData.transformMatrix * vec4(inNormal, 0)).xyz;
    outDistance = (camera.viewMatrix * outPosition).z;
    outTextureIndex = instanceData.textureIndex;
    gl_Position = camera.transformMatrix * outPosition;
}
