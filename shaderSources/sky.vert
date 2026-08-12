#version 460

#extension GL_EXT_scalar_block_layout : require

layout(binding = 2, scalar) uniform CameraBuffer {
	mat4 transformMatrix;
	mat4 viewMatrix;
	vec3 position;
} camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

void main() {
    outUV = inUV;
    gl_Position = camera.transformMatrix * vec4(inPosition + camera.position, 1);
}
