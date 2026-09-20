#version 460

#include "../include/lighting.inc.glsl"

layout(location = 0) in vec4 inColor;

void main() {
	if (ENABLE_DEBUG_LIGHTING) {
		debugLighting();
		return;
	}

    outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV);
	outColor.a = 1.0;
	float fogFactor = clamp((inDistance - fog.start) / (fog.end - fog.start), 0.0, 1.0) * fog.colorAlpha;
	const vec3 lightingColor = MAX_LIGHT_COUNT == 0 ? vec3(1) : getLightingColor(inPosition.xyz, normalize(inNormal), getCascadeIndex(inDistance));
	outColor.rgb = mix(outColor.rgb * inColor.rgb * globalLighting.color.rgb * lightingColor, fog.color, fogFactor);
	outColor.rgb = clamp(outColor.rgb * globalLighting.exposure, 0.0, 1.0);
}
