#version 460

#include "../include/shared.inc.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec2 inLightmapUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in uint inTextureIndex;

layout(location = 0) out vec2 outLightmapUV;
layout(location = 1) out vec3 outPosition;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out float outDistance;
layout(location = 10) out vec2 outUV;
layout(location = 11) flat out uint outTextureIndex;

void main() {
    outLightmapUV = inLightmapUV;
	outPosition = inPosition;
    outNormal = inNormal;
    outDistance = (camera.viewMatrix * vec4(inPosition, 1)).z;
    outUV = inUV;
    outTextureIndex = inTextureIndex;
    gl_Position = camera.transformMatrix * vec4(inPosition, 1);
}
