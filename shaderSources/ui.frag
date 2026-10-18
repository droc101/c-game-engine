#version 460

#include "include/shared.inc.glsl"

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) flat in uint inTextureIndex;

layout(location = 0) out vec4 outColor;

void main() {
    if (inTextureIndex == -1) {
    	outColor = inColor;
    } else {
        const uint index = bitfieldExtract(inTextureIndex, 0, 19);
        const uint flags = bitfieldExtract(inTextureIndex, 29, 3);
        outColor = texture(sampler2D(textures[nonuniformEXT(index)], samplers[(flags & 2) + 2 + (flags >> 2)]), inUV) * inColor;
    }
}
