#version 330 core

in vec2 VERTEX; // Model space vertex position

out vec2 UV; // Output UV coordinates for fragment shader

uniform float height; // Height of the floor

layout (std140) uniform SharedUniforms
{
	mat4 worldViewMatrix;
	vec3 fogColor;
	float fogStart;
	float fogEnd;
	float cameraYaw;
} uniforms;

void main() {
	UV = vec2(VERTEX.x, VERTEX.y);
	gl_Position = uniforms.worldViewMatrix * vec4(VERTEX.x, height, VERTEX.y, 1.0);
}