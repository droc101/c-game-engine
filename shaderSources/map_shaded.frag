#version 460

#include "lighting.inc.glsl"

layout(set = 0, binding = 0) uniform sampler2D lightmap;
layout(set = 0, binding = 1) uniform sampler2D textureSampler[];

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec2 inLightmapUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) flat in uint inTextureIndex;

layout(location = 0) out vec4 outColor;

// w0, w1, w2, and w3 are the four cubic B-spline basis functions
float w0(float a) {
	return (1.0 / 6.0) * (a * (a * (-a + 3.0) - 3.0) + 1.0);
}

float w1(float a) {
	return (1.0 / 6.0) * (a * a * (3.0 * a - 6.0) + 4.0);
}

float w2(float a) {
	return (1.0 / 6.0) * (a * (a * (-3.0 * a + 3.0) + 3.0) + 1.0);
}

float w3(float a) {
	return (1.0 / 6.0) * (a * a * a);
}

// g0 and g1 are the two amplitude functions
float g0(float a) {
	return w0(a) + w1(a);
}

float g1(float a) {
	return w2(a) + w3(a);
}

// h0 and h1 are the two offset functions
float h0(float a) {
	return -1.0 + w1(a) / (w0(a) + w1(a));
}

float h1(float a) {
	return 1.0 + w3(a) / (w2(a) + w3(a));
}

vec4 sampleLightmap(const sampler2D lightmap, const vec2 original_uv){
   vec2 lightmap_size = vec2(textureSize(lightmap, 0));
   vec2 luxel_size = 1.0 / lightmap_size;
   
    vec2 uv = original_uv * lightmap_size - vec2(0.5);

    vec2 iuv = floor(uv);
    vec2 fuv = fract(uv);

    float g0x = g0(fuv.x);
	float g1x = g1(fuv.x);
	float h0x = h0(fuv.x);
	float h1x = h1(fuv.x);
	float h0y = h0(fuv.y);
	float h1y = h1(fuv.y);

    vec2 p0 = (vec2(iuv.x + h0x, iuv.y + h0y) - vec2(0.5)) * luxel_size;
	vec2 p1 = (vec2(iuv.x + h1x, iuv.y + h0y) - vec2(0.5)) * luxel_size;
	vec2 p2 = (vec2(iuv.x + h0x, iuv.y + h1y) - vec2(0.5)) * luxel_size;
	vec2 p3 = (vec2(iuv.x + h1x, iuv.y + h1y) - vec2(0.5)) * luxel_size;

    return (g0(fuv.y) * (g0x * texture(lightmap, p0) + g1x * texture(lightmap, p1))) +
		   (g1(fuv.y) * (g0x * texture(lightmap, p2) + g1x * texture(lightmap, p3)));
}

void main() {
    outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV);
    float fade = clamp(outColor.a, 0.0, 1.0);
    if (fade < 0.001 || fade < fract(MAGIC_Z * fract(dot(gl_FragCoord.xy, MAGIC_XY)))) {
        discard;
    }
	outColor.a = 1.0;
    vec3 lightingColor = getLightingColor(inPosition, normalize(inNormal)) + sampleLightmap(lightmap, inLightmapUV).rgb;
	float fogFactor = clamp((gl_FragCoord.z / gl_FragCoord.w - fog.start) / (fog.end - fog.start), 0.0, 1.0) * fog.colorAlpha;
	outColor.rgb = mix(outColor.rgb * globalLighting.color.rgb * lightingColor, fog.color, fogFactor);
	outColor.rgb = clamp(outColor.rgb * globalLighting.exposure, 0.0, 1.0);
}
