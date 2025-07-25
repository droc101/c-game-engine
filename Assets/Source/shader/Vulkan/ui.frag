#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (binding = 0) uniform sampler2D textureSampler[];

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) flat in uint textureIndex;

layout(location = 0) out vec4 outColor;

void main() {
    if (textureIndex == -1) outColor = inColor;
    else outColor = texture(textureSampler[nonuniformEXT(textureIndex)], inUV) * inColor;
}
