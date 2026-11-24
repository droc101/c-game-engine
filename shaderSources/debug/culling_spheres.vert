#version 460

#include "../include/shared.inc.glsl"

layout(push_constant, scalar) uniform PushConstants {
    uvec2 cullingInfoAddress;
    vec3 color;
    uint isActorModelBuffer;
    uint keptPixelIndexOffset;
} pushConstants;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outWorldPosition;
layout(location = 1) flat out vec3 outCenter;
layout(location = 2) flat out float outRadius;
layout(location = 3) flat out vec3 outColor;
layout(location = 4) flat out uint outKeptPixelIndex;

void main() {
    uint count;
    vec3 center;
    float radius;
    if (pushConstants.isActorModelBuffer != 0) {
        ActorModelCullingInfoBuffer cullingInfoBuffer = ActorModelCullingInfoBuffer(pushConstants.cullingInfoAddress);
        count = cullingInfoBuffer.count;
        if (gl_InstanceIndex < count) {
            center = cullingInfoBuffer.cullingInfos[gl_InstanceIndex].position;
            radius = cullingInfoBuffer.cullingInfos[gl_InstanceIndex].radius;
        }
    } else {
        CullingInfoBuffer cullingInfoBuffer = CullingInfoBuffer(pushConstants.cullingInfoAddress);
        count = cullingInfoBuffer.count;
        if (gl_InstanceIndex < count) {
            center = cullingInfoBuffer.cullingInfos[gl_InstanceIndex].position;
            radius = cullingInfoBuffer.cullingInfos[gl_InstanceIndex].radius;
        }
    }

    if (count <= gl_InstanceIndex) {
        gl_Position = vec4(uintBitsToFloat(0x7fc00000u));
        return;
    }

    outWorldPosition = center + inPosition * radius;
    outCenter = center;
    outRadius = radius;
    outColor = pushConstants.color;
    outKeptPixelIndex = (gl_InstanceIndex + pushConstants.keptPixelIndexOffset) % 4;
    gl_Position = camera.transformMatrix * vec4(outWorldPosition, 1);
}
