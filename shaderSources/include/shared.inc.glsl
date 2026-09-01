// Include only. This file will not compile as a standalone module.

#include "types.inc.glsl"

layout(binding = 2, scalar) readonly restrict uniform CameraBuffer {
	mat4 transformMatrix;
	mat4 viewMatrix;
	vec3 position;
    float nearPlane;
    float farPlane;
    float frustumPlanes[4];
} camera;
