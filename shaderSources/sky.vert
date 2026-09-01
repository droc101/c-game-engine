#version 460

#include "include/shared.inc.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

void main() {
    outUV = inUV;
    gl_Position = camera.transformMatrix * vec4(inPosition + camera.position, 1);
}
