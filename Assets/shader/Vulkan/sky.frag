#version 460
#extension GL_EXT_nonuniform_qualifier: enable

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

layout (binding = 0) uniform sampler2D textureSampler[];

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main() {
	outColor = texture(textureSampler[nonuniformEXT(pushConstants.roofTextureIndex)], inUV);
	if (outColor.a < 0.5) discard;
	outColor.a = 1.0;
}
