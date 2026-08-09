#version 460

#include "shadow_maps_shared.inc.glsl"

layout (binding = 1) uniform sampler2D textureSampler[];

layout(location = 0) in vec3 inUvAlpha;
layout(location = 1) in vec3 inPosition;
layout(location = 2) flat in uint inTextureIndex;

void main() {
    const float alpha = inUvAlpha.z * texture(textureSampler[nonuniformEXT(inTextureIndex)], inUvAlpha.xy).a;
    if (alpha < 0.001 || min(alpha, 1) < fract(MAGIC_Z * fract(dot(gl_FragCoord.xy, MAGIC_XY)))) {
        discard;
    }
}
