// Include only. This file will not compile as a standalone module.

#include "shared.inc.glsl"

layout(constant_id = 0) const uint MAX_LIGHT_COUNT = 1;

const float MIN_BRIGHTNESS = 1.0 / 256.0;

layout(push_constant) uniform PushConstants {
    uint shadowMapSize;
} pushConstants;

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
layout(set = 0, binding = 5, scalar) readonly restrict uniform LightsData {
    float cascadeDepths[4];
	mat4 cascadeMatrices[4];
    // uint lightIndices[MAX_LIGHT_COUNT == 0 ? 1 : MAX_LIGHT_COUNT];
    Light lights[MAX_LIGHT_COUNT == 0 ? 1 : MAX_LIGHT_COUNT];
} lightsData;

layout(set = 0, binding = 6) uniform sampler2DShadow directionalLightShadowMaps[];
layout(set = 1, binding = 0) uniform sampler2DShadow shadowMaps[];

uint getCascadeIndex(const float distance) {
    for (uint cascadeIndex = 0; cascadeIndex < 4; ++cascadeIndex) {
        if (distance < lightsData.cascadeDepths[cascadeIndex]) {
            return cascadeIndex;
        }
    }
    return 4;
}

float getLightBrightness(const Light light, const float distance, const float theta) {
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

    return brightness;
}

const uint SAMPLE_COUNT = 32;

vec2 getSoftShadowKernel(const float sampleIndex) {
    float r = sqrt(sampleIndex + 0.5) / sqrt(float(SAMPLE_COUNT));
    float theta = sampleIndex * 2.4;
    return vec2(cos(theta) * r, sin(theta) * r);
}

float sampleShadowMap(nonuniformEXT sampler2DShadow shadowMap, vec2 uv, const float depth) {
    uv = uv * 0.5 + 0.5;
	if (SAMPLE_COUNT == 0) {
		return texture(shadowMap, vec3(uv, depth));
	}

    const float r = fract(dot(gl_FragCoord.xy, MAGIC)) * 6.283185307179586;
    const float sr = sin(r);
    const float cr = cos(r);
	const mat2 diskRotation = mat2(vec2(cr, -sr), vec2(sr, cr));
    const float size = 4.0 / float(pushConstants.shadowMapSize);

	float sum = 0.0;
    for (uint i = 0; i < SAMPLE_COUNT; i++) {
		sum += texture(shadowMap, vec3(uv + size * (diskRotation * getSoftShadowKernel(float(i))), depth));
	}
    return sum / float(SAMPLE_COUNT);
}

vec3 getLightingColor(const vec3 position, const vec3 normal, const uint cascadeIndex) {
    if (MAX_LIGHT_COUNT == 0) {
        return vec3(1);
    }
    vec3 lightingColor = vec3(0);
    for (uint i = 0; i < MAX_LIGHT_COUNT; i++) {
        if (lightsData.lights[i].type == LIGHT_TYPE_DIRECTIONAL) {
            if (cascadeIndex == 4) {
                lightingColor += lightsData.lights[i].brightness * max(dot(lightsData.lights[i].negativeForwardDirection, normal), 0) * lightsData.lights[i].color;
                continue;
            }
            const vec4 worldPosition = lightsData.cascadeMatrices[cascadeIndex] * vec4(position, 1);
            const vec4 coord = worldPosition / worldPosition.w;
            if (coord.x >= -1 && coord.x <= 1 && coord.y >= -1 && coord.y <= 1) {
                const float factor = sampleShadowMap(directionalLightShadowMaps[nonuniformEXT(cascadeIndex)], coord.xy, coord.z);
                if (factor < 1e-6) {
                    continue;
                }
                lightingColor += factor * lightsData.lights[i].brightness * max(dot(lightsData.lights[i].negativeForwardDirection, normal), 0) * lightsData.lights[i].color;
            }
        } else {
            const vec3 lightToWorld = lightsData.lights[i].transform.position - position;
            const float distance = length(lightToWorld);
            if (distance > lightsData.lights[i].maxDistance) {
                continue;
            }
            const vec3 lightToWorldNormalized = normalize(lightToWorld);
            if (lightsData.lights[i].type == LIGHT_TYPE_SPOT) {
                const float dottedDirection = dot(lightToWorldNormalized, lightsData.lights[i].negativeForwardDirection);
                if (dottedDirection < 0) {
                    continue;
                }
                const float normalFactor = dot(lightToWorldNormalized, normal);
                if (normalFactor < 1e-6) {
                    continue;
                }
                const float theta = degrees(acos(dottedDirection));
                if (theta > lightsData.lights[i].fadingAngle) {
                    continue;
                }
                const vec4 worldPosition = lightsData.lights[i].transformMatrix * vec4(position, 1);
                const vec4 coord = worldPosition / worldPosition.w;
                if (coord.x >= -1 && coord.x <= 1 && coord.y >= -1 && coord.y <= 1) {
                    const float brightness = getLightBrightness(lightsData.lights[i], distance, theta);
                    if (brightness < MIN_BRIGHTNESS) {
                        continue;
                    }
                    const float factor = sampleShadowMap(shadowMaps[nonuniformEXT(lightsData.lights[i].shadowMapIndex)], coord.xy, coord.z);
                    if (factor < 1e-6) {
                        continue;
                    }
                    lightingColor += factor * brightness * normalFactor * lightsData.lights[i].color;
                }
            } else {
                const float normalFactor = dot(lightToWorldNormalized, normal);
                if (normalFactor < 1e-6) {
                    continue;
                }
                const float brightness = getLightBrightness(lightsData.lights[i], distance, 0);
                if (brightness < MIN_BRIGHTNESS) {
                    continue;
                }
                const vec3 lightToWorldAbs = abs(lightToWorld);
                const float scale = max(max(lightToWorldAbs.x, lightToWorldAbs.y), lightToWorldAbs.z);
                const float comparisonDepth = lightsData.lights[i].transformMatrix[2][2] + lightsData.lights[i].transformMatrix[3][2] / (scale - 0.01);
                float factor;
                if (scale == lightToWorldAbs.x) {
                    if (scale == lightToWorld.x) {
                        factor = sampleShadowMap(shadowMaps[nonuniformEXT(lightsData.lights[i].shadowMapIndex)], lightToWorld.zy / -scale, comparisonDepth);
                    } else {
                        factor = sampleShadowMap(shadowMaps[nonuniformEXT(lightsData.lights[i].shadowMapIndex) + 1], vec2(lightToWorld.z, -lightToWorld.y) / scale, comparisonDepth);
                    }
                } else if (scale == lightToWorldAbs.y) {
                    if (scale == lightToWorld.y) {
                        factor = sampleShadowMap(shadowMaps[nonuniformEXT(lightsData.lights[i].shadowMapIndex) + 2], lightToWorld.xz / scale, comparisonDepth);
                    } else {
                        factor = sampleShadowMap(shadowMaps[nonuniformEXT(lightsData.lights[i].shadowMapIndex) + 3], vec2(lightToWorld.x, -lightToWorld.z) / scale, comparisonDepth);
                    }
                } else {
                    if (scale == lightToWorld.z) {
                        factor = sampleShadowMap(shadowMaps[nonuniformEXT(lightsData.lights[i].shadowMapIndex) + 4], vec2(lightToWorld.x, -lightToWorld.y) / scale, comparisonDepth);
                    } else {
                        factor = sampleShadowMap(shadowMaps[nonuniformEXT(lightsData.lights[i].shadowMapIndex) + 5], lightToWorld.xy / -scale, comparisonDepth);
                    }
                }
                if (factor < 1e-6) {
                    continue;
                }
                lightingColor += factor * brightness * normalFactor * lightsData.lights[i].color;
            }
        }
    }
    return lightingColor;
}
