#version 460

layout (push_constant) uniform PushConstants {
	vec3 cameraPosition;
	float yaw;
	mat4 transformMatrix;

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
	gl_Position = pushConstants.transformMatrix * (vec4(inWallVertex, 1.0) + vec4(pushConstants.cameraPosition, 0));
	outUV = inUV;
}