#version 460

#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

const vec2 MAGIC_XY = vec2(0.06711056, 0.00583715);
const float MAGIC_Z = 52.9829189;

layout (binding = 1) uniform sampler2D textureSampler[];

layout(binding = 3, scalar) uniform GlobalLightingBuffer {
	vec4 color;
	float exposure;
} globalLighting;

layout(binding = 4, scalar) uniform FogBuffer {
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
    float fade = clamp(outColor.a * inColor.a, 0.0, 1.0);
    if (fade < 0.001 || fade < fract(MAGIC_Z * fract(dot(gl_FragCoord.xy, MAGIC_XY)))) {
        discard;
    }
	outColor.a = 1.0;
	float fogFactor = clamp((gl_FragCoord.z / gl_FragCoord.w - fog.start) / (fog.end - fog.start), 0.0, 1.0) * fog.colorAlpha;
	outColor.rgb = mix(outColor.rgb * inColor.rgb, fog.color, fogFactor);
	outColor.rgb = clamp(outColor.rgb * globalLighting.exposure, 0.0, 1.0);
}
