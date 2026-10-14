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
layout(set = 0, binding = 1) uniform sampler2D textureSampler[];
#endif

layout(binding = 2, scalar) readonly restrict uniform CameraBuffer {
	mat4 transformMatrix;
	mat4 viewMatrix;
	vec3 position;
    float nearPlane;
    float farPlane;
    float frustumPlanes[4];
} camera;

const vec2 MAGIC = vec2(0.06711056, 0.00583715) * 52.9829189;
