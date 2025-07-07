#version 330 core

in vec3 VERTEX; // Model space vertex position
in vec2 VERTEX_UV; // Vertex UV coordinates

out vec2 UV; // Output UV coordinates for fragment shader

uniform mat4 MODEL_WORLD_MATRIX; // Model to world matrix

layout (std140) uniform SharedUniforms
{
	mat4 worldViewMatrix;
	vec3 fogColor;
	float fogStart;
	float fogEnd;
	float cameraYaw;
} uniforms;

const vec2 UVs[] = vec2[](
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 1.0)
);

void main() {
	UV = UVs[gl_VertexID];
	gl_Position = uniforms.worldViewMatrix * MODEL_WORLD_MATRIX * vec4(VERTEX.x, -0.49, VERTEX.y, 1.0);
}
