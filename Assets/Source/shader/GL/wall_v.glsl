#version 330 core

in vec3 VERTEX; // Model space vertex position
in vec2 VERTEX_UV; // Vertex UV coordinates
in float VERTEX_ANGLE;

out vec2 UV; // Output UV coordinates for fragment shader
flat out float SHADE; // Output shade value for fragment shader

layout (std140) uniform SharedUniforms
{
	mat4 worldViewMatrix;
	vec3 fogColor;
	float fogStart;
	float fogEnd;
	float cameraYaw;
} uniforms;

const float PI = 3.14159265359; // yummy

void main() {
	UV = VERTEX_UV;
	gl_Position = uniforms.worldViewMatrix * vec4(VERTEX, 1.0);

	SHADE = abs(cos((uniforms.cameraYaw + (1.5 * PI)) - VERTEX_ANGLE));
	SHADE = max(0.6, min(1, SHADE));
}