// Include only. This file will not compile as a standalone module.

#include "shared.inc.glsl"

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
layout(set = 0, binding = 5, scalar) readonly restrict buffer LightsData {
    uint lightCount;
	mat4 cascades[4];
    Light lights[];
} lightsData;

layout(set = 1, binding = 0) uniform sampler2DShadow spotLightShadowMaps[];
layout(set = 2, binding = 0) uniform samplerCubeShadow pointLightShadowMaps[];

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

vec3 getLightingColor(const vec3 position, const vec3 normal) {
    if (lightsData.lightCount == 0) {
        return vec3(1);
    }
    vec3 lightingColor = vec3(0);
    for (uint i = 0; i < lightsData.lightCount; i++) {
        const Light light = lightsData.lights[i];
        if (light.type == LIGHT_TYPE_SPOT) {
            const vec4 worldPosition = light.transformMatrix * vec4(position, 1);
            const vec4 coord = worldPosition / worldPosition.w;
            if (coord.x >= -1 && coord.x <= 1 && coord.y >= -1 && coord.y <= 1) {
                const vec3 lightToWorld = light.position - position;
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
                lightingColor += factor * getLightColor(light, length(lightToWorld), theta) * max(dot(lightToWorldNormalized, normal), 0);
            }
        } else if (light.type == LIGHT_TYPE_DIRECTIONAL) {
            const vec4 worldPosition = light.transformMatrix * vec4(position, 1);
            const vec4 coord = worldPosition / worldPosition.w;
            if (coord.x >= -1 && coord.x <= 1 && coord.y >= -1 && coord.y <= 1) {
                const float factor = texture(spotLightShadowMaps[light.shadowMapIndex], vec3(coord.xy * 0.5 + 0.5, coord.z));
                if (factor < 1e-6) {
                    continue;
                }
                lightingColor += factor * light.color * light.brightness * max(dot(light.negativeForwardDirection, normal), 0);
            } else {
                // TODO: Remove once CSM is implemented
                lightingColor += light.color * light.brightness * max(dot(light.negativeForwardDirection, normal), 0);
            }
        } else {
            const vec3 lightToWorld = light.position - position;
            const float depth = length(lightToWorld);
            const float factor = texture(pointLightShadowMaps[light.shadowMapIndex], vec4(lightToWorld, depth));
            if (factor < 1e-6) {
                continue;
            }
            lightingColor += getLightColor(light, depth, 0) * max(dot(normalize(lightToWorld), normal), 0);
        }
    }
    return lightingColor;
}
