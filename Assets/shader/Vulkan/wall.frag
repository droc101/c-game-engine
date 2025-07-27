#version 460
#extension GL_EXT_nonuniform_qualifier: enable

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

layout (binding = 0) uniform sampler2D textureSampler[];

layout (location = 0) in vec2 inUV;
layout (location = 1) flat in uint inTextureIndex;
layout (location = 2) flat in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main() {
	outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV);
	outColor *= inColor.a;
	if (outColor.a < 0.5) discard;
	outColor.a = 1.0;
	outColor.rgb = mix(outColor.rgb, inColor.rgb, clamp((gl_FragCoord.z / gl_FragCoord.w - pushConstants.fogStart) / (pushConstants.fogEnd - pushConstants.fogStart), 0.0, 1.0));
}
