#version 460

layout (push_constant) uniform PushConstants {
	vec2 playerPosition;
	float yaw;
	mat4 transformMatrix;

	uint skyVertexCount;
	uint skyTextureIndex;

	float fogStart;
	float fogEnd;
	uint fogColor;
} pushConstants;

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inUV;
layout(location = 2) in mat4 inTransform;
layout(location = 6) in uint inTextureIndex;
layout(location = 7) in vec4 inColor;

layout(location = 0) out vec2 outUV;
layout(location = 1) flat out uint outTextureIndex;
layout(location = 2) out vec4 outColor;

void main() {
	outUV = inUV;
	outTextureIndex = inTextureIndex;
	outColor = inColor;
	gl_Position = pushConstants.transformMatrix * inTransform * vec4(inVertex, 1.0);
}