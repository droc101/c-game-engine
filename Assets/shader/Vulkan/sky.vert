#version 460

layout (push_constant) uniform PushConstants {
	vec2 playerPosition;
	float yaw;
	mat4 translationMatrix;

	uint roofTextureIndex;
	uint floorTextureIndex;

	float fogStart;
	float fogEnd;
	uint fogColor;
} pushConstants;

layout (location = 0) in vec3 inWallVertex;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec2 outUV;

void main() {
	gl_Position = pushConstants.translationMatrix * (vec4(inWallVertex, 1.0) + vec4(pushConstants.playerPosition.x, 0, pushConstants.playerPosition.y, 0));
	outUV = inUV;
}