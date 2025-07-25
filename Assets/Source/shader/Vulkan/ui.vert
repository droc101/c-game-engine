#version 460

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;
layout(location = 3) in uint inTextureIndex;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) out uint outTextureIndex;

void main() {
    gl_Position = vec4(inPos, 0.0, 1.0);
    outColor = inColor;
    outUV = inUV;
    outTextureIndex = inTextureIndex;
}