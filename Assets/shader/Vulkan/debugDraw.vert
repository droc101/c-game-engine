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

layout (location = 0) in vec3 inVertex;
layout (location = 1) in vec4 inColor;

layout (location = 0) flat out vec4 outColor;

void main() {
	outColor = inColor;
	gl_Position = pushConstants.transformMatrix * vec4(inVertex, 1.0);
}