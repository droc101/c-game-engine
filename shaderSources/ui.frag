#version 460

#include "include/shared.inc.glsl"

layout(binding = 1) uniform sampler2D textureSampler[];

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) flat in uint textureIndex;

layout(location = 0) out vec4 outColor;

void main() {
    if (textureIndex == -1) {
    	outColor = inColor;
    } else {
        vec2 uv = inUV + (1.0 / (100 * textureSize(textureSampler[nonuniformEXT(textureIndex)], 0)));
    	outColor = texture(textureSampler[nonuniformEXT(textureIndex)], uv) * inColor;
    }
}
