#version 460

#define UNSHADED
#include "../include/model_fragment.inc.glsl"

layout(location = 0) in vec4 inColor;

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

bool debugLighting() {
    if (DEBUG_RENDERING == DEBUG_RENDERING_DISABLED) {
        return false;
    }
    outColor = vec4(0, 0, 0, 1);
    switch (DEBUG_RENDERING) {
        case DEBUG_RENDERING_DISABLE_LIGHTING:
        case DEBUG_RENDERING_UNTEXTURED:
        case DEBUG_RENDERING_ONLY_LIGHTING:
        case DEBUG_RENDERING_NO_LIGHT_FALLOFF:
        case DEBUG_RENDERING_NORMALS:
        case DEBUG_RENDERING_SHOW_CLUSTERS:
        case DEBUG_RENDERING_SHOW_CLUSTER_LIGHT_COUNTS:
        default:
            return false;
        case DEBUG_RENDERING_UVS:
        {
            outColor.rg = mod(inUV, 1);
            return true;
        }
        case DEBUG_RENDERING_DEPTH_ONLY:
			outColor.rgb = vec3(((1 - gl_FragCoord.z) / gl_FragCoord.w) / (camera.farPlane - camera.nearPlane));
            return true;
    }
}

void main() {
	if (debugLighting()) {
		return;
	}
	
	getTextureColor();
	float fogFactor = clamp(((1 - gl_FragCoord.z) / gl_FragCoord.w - fog.start) / (fog.end - fog.start), 0.0, 1.0) * fog.colorAlpha;
	outColor.rgb = mix(outColor.rgb * inColor.rgb, fog.color, fogFactor);
	outColor.rgb = clamp(outColor.rgb * globalLighting.exposure, 0.0, 1.0);
}
