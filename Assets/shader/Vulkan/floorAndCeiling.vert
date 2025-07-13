#version 460

const vec2 VERTICES[6] = vec2[](
	vec2(-100, 100),
	vec2(100, 100),
	vec2(100, -100),
	vec2(-100, 100),
	vec2(100, -100),
	vec2(-100, -100)
);

layout(push_constant) uniform PushConstants
{
	vec2 playerPosition;
	float yaw;
	mat4 translationMatrix;

	uint roofTextureIndex;
	uint floorTextureIndex;

	float fogStart;
	float fogEnd;
	uint fogColor;
}
pushConstants;

layout(location = 0) out vec2 outUV;
layout(location = 1) flat out uint outTextureIndex;

void main()
{
	vec2 vertex = VERTICES[gl_VertexIndex % 6] + pushConstants.playerPosition;
	outUV = vertex;
	if (gl_VertexIndex < 6) {
		outTextureIndex = pushConstants.floorTextureIndex;
		gl_Position = pushConstants.translationMatrix * vec4(vertex.x, -0.5, vertex.y, 1);
	} else {
		outTextureIndex = pushConstants.roofTextureIndex;
		gl_Position = pushConstants.translationMatrix * vec4(vertex.x, 0.5, vertex.y, 1);
	}
}
