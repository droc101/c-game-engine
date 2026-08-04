#version 460

#extension GL_EXT_scalar_block_layout : require

layout(binding = 2, scalar) uniform TransformMatrixBuffer {
	mat4 matrix;
} transform;

layout (location = 0) in vec3 inVertex;
layout (location = 1) in vec4 inColor;

layout (location = 0) flat out vec4 outColor;

void main() {
	outColor = inColor;
	gl_Position = transform.matrix * vec4(inVertex, 1.0);
}

