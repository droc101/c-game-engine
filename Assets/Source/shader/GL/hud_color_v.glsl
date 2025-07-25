#version 330 core

/**
 * This shader is used to render a simple colored quad.
 */

// Input from OpenGL
in vec2 VERTEX;

void main()
{
	gl_Position = vec4(VERTEX, 0.0, 1.0);
}
