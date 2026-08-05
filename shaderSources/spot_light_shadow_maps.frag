#version 460

#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

const vec2 MAGIC_XY = vec2(0.06711056, 0.00583715);
const float MAGIC_Z = 52.9829189;

layout (binding = 1) uniform sampler2D textureSampler[];

layout(location = 0) in vec4 inUvAlphaDistance;
layout(location = 1) flat in uint inTextureIndex;

void main() {
    const float alpha = inUvAlphaDistance.z * texture(textureSampler[nonuniformEXT(inTextureIndex)], inUvAlphaDistance.xy).a;
    if (alpha < 0.001 || min(alpha, 1) < fract(MAGIC_Z * fract(dot(gl_FragCoord.xy, MAGIC_XY)))) {
        discard;
    }
}
