#version 460

#include "shared.inc.glsl"

layout(set = 0, binding = 0) uniform sampler2D lightmap;
layout(set = 0, binding = 1) uniform sampler2D textureSampler[];
layout(set = 0, binding = 3, scalar) readonly restrict uniform GlobalLightingBuffer {
	vec4 color;
	float exposure;
} globalLighting;
layout(set = 0, binding = 4, scalar) readonly restrict uniform FogBuffer {
	vec3 color;
	float colorAlpha;
	float start;
	float end;
} fog;
layout(set = 0, binding = 5, scalar) readonly restrict buffer LightsData {
    uint lightCount;
    Light lights[];
} lightsData;

layout(set = 1, binding = 0) uniform sampler2DShadow spotLightShadowMaps[];
layout(set = 2, binding = 0) uniform samplerCubeShadow pointLightShadowMaps[];

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

vec3 getLightColor(const Light light, const float distance, const float theta) {
    float multiplierSquared;
    float brightness;
    if (light.type == LIGHT_TYPE_DIRECTIONAL) {
        brightness = light.brightness;
    } else {
        multiplierSquared = light.attenuationMultiplier * light.attenuationMultiplier;
        brightness = (multiplierSquared * light.brightness) /
            (multiplierSquared * light.constantAttenuation +
                light.attenuationMultiplier * light.linearAttenuation * distance +
                light.quadraticAttenuation * distance * distance);
        if (light.type == LIGHT_TYPE_SPOT) {
            if (theta < light.brightAngle) {
                brightness *= 0.75 + 0.25 * (light.brightAngle - theta) / light.brightAngle;
            } else {
                brightness *= 0.75 * (light.fadingAngle - theta) / (light.fadingAngle - light.brightAngle);
            }
        }
    } 

    if (brightness < MIN_BRIGHTNESS) {
        return vec3(0);
    }
    return light.color * brightness;
}

void main() {
    vec3 lightingColor = vec3(0);
    for (uint i = 0; i < lightsData.lightCount; i++) {
        const Light light = lightsData.lights[i];
        if (light.type != LIGHT_TYPE_POINT && light.type != LIGHT_TYPE_SPOT) {
            continue;
        }
        if (light.type == LIGHT_TYPE_SPOT) {
            const vec4 position = light.transformMatrix * vec4(inPosition, 1);
            const vec4 coord = position / position.w;
            if (coord.x >= -1 &&
                coord.x <= 1 &&
                coord.y >= -1 &&
                coord.y <= 1) {
                    const vec3 lightToWorld = light.position - inPosition.xyz;
                    const vec3 lightToWorldNormalized = normalize(lightToWorld);
                    const float dottedDirection = dot(lightToWorldNormalized, light.negativeForwardDirection);
                    if (dottedDirection < 0) {
                        continue;
                    }
                    const float theta = degrees(acos(dottedDirection));
                    if (theta > light.fadingAngle) {
                        continue;
                    }
                    const float factor = texture(spotLightShadowMaps[light.shadowMapIndex], vec3(coord.xy * 0.5 + 0.5, coord.z)); 
                    lightingColor += factor * getLightColor(light, length(lightToWorld), theta) * max(dot(lightToWorldNormalized, normalize(inNormal)), 0);
            }
        } else {
            const vec3 lightToWorld = light.position - inPosition;
            const float depth = length(lightToWorld);
            const float factor = texture(pointLightShadowMaps[light.shadowMapIndex], vec4(lightToWorld, depth));
            if (factor < 1e-6) {
                continue;
            }
            lightingColor += factor * getLightColor(light, depth, 0);// * max(dot(normalize(lightToWorld), normalize(inNormal)), 0);
        }
    }

    outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV);
    float fade = clamp(outColor.a, 0.0, 1.0);
    if (fade < 0.001 || fade < fract(MAGIC_Z * fract(dot(gl_FragCoord.xy, MAGIC_XY)))) {
        discard;
    }
	outColor.a = 1.0;
    // lightingColor += sampleLightmap(lightmap, inLightmapUV).rgb;
	float fogFactor = clamp((gl_FragCoord.z / gl_FragCoord.w - fog.start) / (fog.end - fog.start), 0.0, 1.0) * fog.colorAlpha;
	outColor.rgb = mix(outColor.rgb * globalLighting.color.rgb * lightingColor, fog.color, fogFactor);
	outColor.rgb = clamp(outColor.rgb * globalLighting.exposure, 0.0, 1.0);
}
