#version 460

#include "../include/shared.inc.glsl"

layout(binding = 1) uniform sampler2D textureSampler[];

layout(binding = 3, scalar) readonly restrict uniform GlobalLightingBuffer {
	vec4 color;
	float exposure;
} globalLighting;

layout(binding = 4, scalar) readonly restrict uniform FogBuffer {
	vec3 color;
	float colorAlpha;
	float start;
	float end;
} fog;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) flat in uint inTextureIndex;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV);
	outColor.a = 1.0;
	float fogFactor = clamp((gl_FragCoord.z / gl_FragCoord.w - fog.start) / (fog.end - fog.start), 0.0, 1.0) * fog.colorAlpha;
	outColor.rgb = mix(outColor.rgb * inColor.rgb, fog.color, fogFactor);
	outColor.rgb = clamp(outColor.rgb * globalLighting.exposure, 0.0, 1.0);
}
