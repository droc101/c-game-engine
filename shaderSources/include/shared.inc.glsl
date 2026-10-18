// Include only. This file will not compile as a standalone module.

#include "types.inc.glsl"

#if defined(SHADER_TYPE_FRAGMENT) || defined(IDE)
#   ifndef DEPTH_ONLY
#       ifdef IDE
#           define DEBUG_RENDERING 0
#       else
layout(constant_id = 0) const uint DEBUG_RENDERING = 0;
#       endif
#   endif
layout(set = 0, binding = 0) uniform texture2D textures[];
layout(set = 0, binding = 1) uniform sampler samplers[6];
vec4 sampleTexture(const uint textureIndex, const vec2 uv) {
    const uint index = bitfieldExtract(textureIndex, 0, 19);
    const uint flags = bitfieldExtract(textureIndex, 29, 3);
    if ((flags & 1) == 0) {
        return texture(sampler2D(textures[nonuniformEXT(index)], samplers[(flags & 2) + 2 + (flags >> 2)]), uv);
    }
    if ((flags & 4) == 0) {
        const ivec2 size = textureSize(sampler2D(textures[nonuniformEXT(textureIndex)], samplers[flags >> 1]), 0);
        const vec2 scaledUV = fract(uv) * size + 0.5;
        const vec2 adjustedUV = (ivec2(scaledUV) + clamp((fract(scaledUV) - 0.5) / fwidth(scaledUV), -0.5, 0.5)) / size;
        return textureGrad(sampler2D(textures[nonuniformEXT(textureIndex)], samplers[flags >> 1]), adjustedUV, dFdx(uv), dFdy(uv));
    }
    return texture(sampler2D(textures[nonuniformEXT(index)], samplers[((flags & 2) >> 1) ^ 1]), uv);
}
#endif

layout(binding = 3, scalar) readonly restrict uniform CameraBuffer {
	mat4 transformMatrix;
	mat4 viewMatrix;
	vec3 position;
    float nearPlane;
    float farPlane;
    float frustumPlanes[4];
} camera;

const vec2 MAGIC = vec2(0.06711056, 0.00583715) * 52.9829189;
