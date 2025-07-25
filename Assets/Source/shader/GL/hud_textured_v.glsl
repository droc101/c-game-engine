#version 330 core

/**
 * This shader is used to render a textured quad with a tint and optional region.
 */

// Input from OpenGL
in vec2 VERTEX;
in vec2 VERTEX_UV;

// Output to fragment shader
out vec2 UV;

void main()
{
	UV = VERTEX_UV;
	gl_Position = vec4(VERTEX, 0.0, 1.0);
}
