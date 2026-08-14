#version 460

#include "shadow_maps_shared.inc.glsl"

layout(binding = 1) uniform sampler2D textureSampler[];

layout(location = 0, component = 0) in vec2 inUv;
layout(location = 0, component = 3) in float inAlpha;
layout(location = 1) in vec3 inPosition;
layout(location = 2) flat in uint inTextureIndex;

layout(location = 0) out float outDepth;

void main() {
    const float alpha = inAlpha * texture(textureSampler[nonuniformEXT(inTextureIndex)], inUv).a;
    if (alpha < 0.001 || min(alpha, 1) < fract(MAGIC_Z * fract(dot(gl_FragCoord.xy, MAGIC_XY)))) {
        discard;
    }
    outDepth = length(inPosition - lightsData.lights[pushConstants.lightIndex].transform.position);
}
