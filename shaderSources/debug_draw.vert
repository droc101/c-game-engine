#version 460

#include "include/shared.inc.glsl"

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec4 inColor;

layout(location = 0) flat out vec4 outColor;

void main() {
	outColor = inColor;
	gl_Position = camera.transformMatrix * vec4(inVertex, 1.0);
}
