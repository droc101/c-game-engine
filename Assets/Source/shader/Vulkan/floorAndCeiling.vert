#version 460

const vec2 VERTICES[12] = vec2[](
	// Floor
	vec2(-100, 100),
	vec2(100, 100),
	vec2(100, -100),
	vec2(-100, 100),
	vec2(100, -100),
	vec2(-100, -100),
	// Ceiling
	vec2(100, 100),
	vec2(-100, 100),
	vec2(-100, -100),
	vec2(100, 100),
	vec2(-100, -100),
	vec2(100, -100)
);

layout(push_constant) uniform PushConstants
{
	vec2 playerPosition;
	float yaw;
	mat4 transformMatrix;

	uint roofTextureIndex;
	uint floorTextureIndex;

	float fogStart;
	float fogEnd;
	uint fogColor;
}
pushConstants;

layout(location = 0) out vec2 outUV;
layout(location = 1) flat out uint outTextureIndex;
layout(location = 2) flat out float outMultiplier;

void main()
{
	vec2 vertex = VERTICES[gl_VertexIndex] + pushConstants.playerPosition;
	outUV = vertex;
	if (gl_VertexIndex < 6) {
		outTextureIndex = pushConstants.floorTextureIndex;
		outMultiplier = 1;
		gl_Position = pushConstants.transformMatrix * vec4(vertex.x, -0.5, vertex.y, 1);
	} else {
		outTextureIndex = pushConstants.roofTextureIndex;
		outMultiplier = 0.8;
		gl_Position = pushConstants.transformMatrix * vec4(vertex.x, 0.5, vertex.y, 1);
	}
}
