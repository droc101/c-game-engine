#version 460

layout (push_constant) uniform PushConstants {
	vec2 playerPosition;
	float yaw;
	mat4 translationMatrix;

	uint skyVertexCount;
	uint skyTextureIndex;

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

const uint WALLS_INSTANCE_INDEX = 0x57414C4C;

void main() {
	outColor = vec4(unpackUnorm4x8(pushConstants.fogColor).bgr, 1);
	if (gl_InstanceIndex == WALLS_INSTANCE_INDEX) {
		// Walls
		gl_Position = pushConstants.translationMatrix * vec4(inWallVertex, 1.0);
		outUV = inUV;
		outTextureIndex = inTextureIndex;
		outColor.a = max(0.6, min(1, abs(cos(pushConstants.yaw - inWallAngle))));
		return;
	}
	if (inTextureIndex == pushConstants.skyTextureIndex)
	{
		// Sky
		gl_Position = pushConstants.translationMatrix * (vec4(inWallVertex, 1.0) + vec4(pushConstants.playerPosition.x, 0, pushConstants.playerPosition.y, 0));
		outUV = inUV;
		outTextureIndex = inTextureIndex;
		return;
	} else {
		// Floor and Ceiling
		gl_Position = pushConstants.translationMatrix * (vec4(inWallVertex, 1.0) + vec4(pushConstants.playerPosition.x, 0, pushConstants.playerPosition.y, 0));
		outUV = inUV + pushConstants.playerPosition;
		outTextureIndex = inTextureIndex;
		if (pushConstants.skyVertexCount == 0 && gl_VertexIndex < 8 && 4 <= gl_VertexIndex) {
			// Ceiling
			outColor.a = 0.8;
		}
		return;
	}
}