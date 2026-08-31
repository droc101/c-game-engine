// Include only. This file will not compile as a standalone module.

#include "shared.inc.glsl"

layout(constant_id = 0) const uint LIGHT_COUNT = 1;

const float MIN_BRIGHTNESS = 1.0 / 256.0;

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
    Light lights[LIGHT_COUNT];
} lightsData;

layout(set = 0, binding = 6) uniform sampler2DShadow directionalLightShadowMaps[];
layout(set = 1, binding = 0) uniform sampler2DShadow spotLightShadowMaps[];
layout(set = 2, binding = 0) uniform samplerCubeShadow pointLightShadowMaps[];

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

vec3 getLightingColor(const vec3 position, const vec3 normal, const float distance) {
    if (LIGHT_COUNT == 0) {
        return vec3(1);
    }
    vec3 lightingColor = vec3(0);
    for (uint i = 0; i < LIGHT_COUNT; i++) {
        const Light light = lightsData.lights[i];
        if (light.type == LIGHT_TYPE_SPOT) {
            const vec4 worldPosition = light.transformMatrix * vec4(position, 1);
            const vec4 coord = worldPosition / worldPosition.w;
            if (coord.x >= -1 && coord.x <= 1 && coord.y >= -1 && coord.y <= 1) {
                const vec3 lightToWorld = light.transform.position - position;
                const vec3 lightToWorldNormalized = normalize(lightToWorld);
                const float dottedDirection = dot(lightToWorldNormalized, light.negativeForwardDirection);
                if (dottedDirection < 0) {
                    continue;
                }
                const float theta = degrees(acos(dottedDirection));
                if (theta > light.fadingAngle) {
                    continue;
                }
                const float factor = texture(spotLightShadowMaps[nonuniformEXT(light.shadowMapIndex)], vec3(coord.xy * 0.5 + 0.5, coord.z)); 
                lightingColor += factor * getLightBrightness(light, length(lightToWorld), theta) * max(dot(lightToWorldNormalized, normal), 0) * light.color;
            }
        } else if (light.type == LIGHT_TYPE_DIRECTIONAL) {
            uint cascadeIndex;
            for (cascadeIndex = 0; cascadeIndex < 4; ++cascadeIndex) {
                if (distance < lightsData.cascadeDepths[cascadeIndex]) {
                    break;
                }
            }
            if (cascadeIndex == 4) {
                lightingColor += 1;
                continue;
            }
            const vec4 worldPosition = lightsData.cascadeMatrices[cascadeIndex] * vec4(position, 1);
            const vec4 coord = worldPosition / worldPosition.w;
            if (coord.x >= -1 && coord.x <= 1 && coord.y >= -1 && coord.y <= 1) {
                const float factor = texture(directionalLightShadowMaps[nonuniformEXT(cascadeIndex)], vec3(coord.xy * 0.5 + 0.5, coord.z));
                if (factor < 1e-6) {
                    continue;
                }
                lightingColor += factor * light.brightness * max(dot(light.negativeForwardDirection, normal), 0) * light.color;
            }
        } else {
            const vec3 lightToWorld = light.transform.position - position;
            const float normalFactor = max(dot(normalize(lightToWorld), normal), 0);
            if (normalFactor < 1e-6) {
                continue;
            }
            const float brightness = getLightBrightness(light, length(lightToWorld), 0);
            if (brightness < MIN_BRIGHTNESS) {
                continue;
            }
            const float scale = max(max(abs(lightToWorld.x), abs(lightToWorld.y)), abs(lightToWorld.z));
            const float comparisonDepth = light.transformMatrix[2][2] + light.transformMatrix[3][2] / scale;
            const float factor = texture(pointLightShadowMaps[nonuniformEXT(light.shadowMapIndex)], vec4(lightToWorld, comparisonDepth));
            if (factor < 1e-6) {
                continue;
            }
            lightingColor += factor * brightness * normalFactor * light.color;
        }
    }
    return lightingColor;
}
