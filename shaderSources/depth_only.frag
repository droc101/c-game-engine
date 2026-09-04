#version 460

#include "include/shadow_maps_shared.inc.glsl"

layout(binding = 1) uniform sampler2D textureSampler[];

layout(location = 0, component = 0) in vec2 inUv;
layout(location = 0, component = 3) in float inAlpha;
layout(location = 1) flat in uint inTextureIndex;

void main() {
    // Contrary to what one might expect, it is faster to sample the texture FIRST here, rather than last, since the memory read latency gets better hidden.
    // This is not normally required as the driver will typically hoist the texture call to be as early as possible, but in this case it fails.
    // From what I can tell both the AMD and NVIDIA drivers won't hoist the fetch to before the branch, but profiling shows that manually hoisting it helps performance.
    const float textureAlpha = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUv).a;

    // We intentionally calculate this even before the inAlpha < 0.001 check to reduce wasted cycles waiting for the input variables to actually be available.
    // The wait will happen either way, and most fragments will not have an inAlpha value less than 1, let alone 0.001.
    // Therefore, calculating the cutoff before the check allows us to use the ALU while we're waiting for inAlpha.
    const float cutoff = fract(dot(gl_FragCoord.xy, MAGIC));
    if (inAlpha < 0.001 || inAlpha < cutoff) {
        discard;
    }

    // Finally we read the texture, and so this line will have the most samples of the whole shader, since our texture read wait happens here
    const float alpha = inAlpha * textureAlpha;
    if (alpha < 0.001 || alpha < cutoff) {
        discard;
    }
}
