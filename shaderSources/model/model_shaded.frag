#version 460

#include "../include/lighting.inc.glsl"

layout(set = 0, binding = 1) uniform sampler2D textureSampler[];

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
	float fogFactor = clamp((gl_FragCoord.z / gl_FragCoord.w - fog.start) / (fog.end - fog.start), 0.0, 1.0) * fog.colorAlpha;
	outColor.rgb = mix(outColor.rgb * inColor.rgb * globalLighting.color.rgb * getLightingColor(inPosition.xyz, normalize(inNormal), getCascadeIndex(inDistance)), fog.color, fogFactor);
	outColor.rgb = clamp(outColor.rgb * globalLighting.exposure, 0.0, 1.0);
}
