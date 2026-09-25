#version 460

#include "../include/lighting.inc.glsl"

layout(location = 0) in vec4 inColor;

void main() {
	if (debugLighting()) {
		return;
	}

	getTextureColor();
	// TODO: Does this branch get compiled out
	vec3 lightingColor = LIGHT_COUNT == 0 ? vec3(1) : getLightingColor(inPosition.xyz, normalize(inNormal), getCascadeIndex(inDistance));
	if (DEBUG_RENDERING == DEBUG_RENDERING_ONLY_LIGHTING || DEBUG_RENDERING == DEBUG_RENDERING_NO_LIGHT_FALLOFF) {
		outColor.rgb = clamp(lightingColor * globalLighting.exposure, 0.0, 1.0);
		outColor.a = 1.0;
		return;
	}
	if (DEBUG_RENDERING == DEBUG_RENDERING_DISABLE_LIGHTING) {
		lightingColor = vec3(1);
	}
	float fogFactor = clamp((inDistance - fog.start) / (fog.end - fog.start), 0.0, 1.0) * fog.colorAlpha;
	outColor.rgb = mix(outColor.rgb * inColor.rgb * globalLighting.color.rgb * lightingColor, fog.color, fogFactor);
	outColor.rgb = clamp(outColor.rgb * globalLighting.exposure, 0.0, 1.0);
}
