#version 460

#include "../include/shared.inc.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in uint inTextureIndex;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) flat out uint outTextureIndex;

void main() {
    outColor = vec4(1);
    outUV = inUV;
    outTextureIndex = inTextureIndex;
    gl_Position = camera.transformMatrix * vec4(inPosition, 1);
}
