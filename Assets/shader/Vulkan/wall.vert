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
layout (location = 2) in uint inTextureIndex;
layout (location = 3) in float inWallAngle;

layout (location = 0) out vec2 outUV;
layout (location = 1) flat out uint outTextureIndex;
layout (location = 2) flat out vec4 outColor;

void main() {
	outUV = inUV;
	outTextureIndex = inTextureIndex;
	outColor = vec4(unpackUnorm4x8(pushConstants.fogColor).bgr, 1);
	outColor.a = max(0.6, abs(cos(pushConstants.yaw - inWallAngle)));
	gl_Position = pushConstants.transformMatrix * vec4(inWallVertex, 1.0);
}