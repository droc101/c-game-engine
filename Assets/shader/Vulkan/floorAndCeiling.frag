#version 460
#extension GL_EXT_nonuniform_qualifier: enable

layout (binding = 0) uniform sampler2D textureSampler[];

layout(location = 0) in vec2 inUV;
layout(location = 1) flat in uint inTextureIndex;
layout(location = 2) flat in float inMultiplier;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV);
	if (outColor.a < 0.5) discard;
	outColor.rgb *= inMultiplier;
	outColor.a = 1.0;
}
