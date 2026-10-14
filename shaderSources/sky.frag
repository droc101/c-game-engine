#version 460

#include "include/shared.inc.glsl"

layout(binding = 4, scalar) readonly restrict uniform GlobalLightingBuffer {
	vec4 color;
	float exposure;
} globalLighting;

layout(push_constant, scalar) uniform PushConstants {
	uint textureIndex;
} pushConstants;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
   	outColor.rgb = clamp(sampleTexture(pushConstants.textureIndex, inUV).rgb * globalLighting.exposure, 0.0, 1.0);
}
