#version 460

#include "shared.inc.glsl"

layout(binding = 1) uniform sampler2D textureSampler[];

layout(binding = 3, scalar) readonly restrict uniform GlobalLightingBuffer {
	vec4 color;
	float exposure;
} globalLighting;

layout(push_constant, scalar) uniform PushConstants {
	uint textureIndex;
} pushConstants;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(textureSampler[nonuniformEXT(pushConstants.textureIndex)], inUV);
   	outColor.rgb = clamp(outColor.rgb * globalLighting.exposure, 0.0, 1.0);
}
