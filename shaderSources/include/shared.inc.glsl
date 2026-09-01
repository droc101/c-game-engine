// Include only. This file will not compile as a standalone module.

#include "types.inc.glsl"

const vec2 MAGIC_XY = vec2(0.06711056, 0.00583715);
const float MAGIC_Z = 52.9829189;

layout(binding = 2, scalar) readonly restrict uniform CameraBuffer {
	mat4 transformMatrix;
	mat4 viewMatrix;
	vec3 position;
    float nearPlane;
    float farPlane;
    float frustumPlanes[4];
} camera;
