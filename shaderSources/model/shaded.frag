#version 460

#include "../include/lighting.inc.glsl"

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in float inDistance;
layout(location = 5) flat in uint inTextureIndex;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV);
	outColor.a = 1.0;
	float fogFactor = clamp((inDistance - fog.start) / (fog.end - fog.start), 0.0, 1.0) * fog.colorAlpha;
	const vec3 lightingColor = MAX_LIGHT_COUNT == 0 ? vec3(1) : getLightingColor(inPosition.xyz, normalize(inNormal), getCascadeIndex(inDistance));
	outColor.rgb = mix(outColor.rgb * inColor.rgb * globalLighting.color.rgb * lightingColor, fog.color, fogFactor);
	outColor.rgb = clamp(outColor.rgb * globalLighting.exposure, 0.0, 1.0);
}
