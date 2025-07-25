#version 330 core

/**
 * This shader is used to render a simple colored quad.
 */

// Input from vertex shader
in vec2 UV;

// Output to fragment shader
out vec4 COLOR;

// Uniforms
uniform vec4 col;

void main() {
	COLOR = col;
}