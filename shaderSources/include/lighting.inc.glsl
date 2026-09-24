// Include only. This file will not compile as a standalone module.

#include "shared.inc.glsl"

#define USE_CLUSTERED

layout(constant_id = 0) const uint LIGHT_COUNT = 1;
layout(constant_id = 1) const uint SAMPLE_COUNT = 32;
layout(constant_id = 2) const float SAMPLE_RADIUS = 4.0;

layout(constant_id = 3) const bool ENABLE_BAKED_LIGHTING = true;
layout(constant_id = 4) const bool ENABLE_CLUSTER_DEBUG = false;

/// Set to true if any of the debug lighting paths should be used
const bool ENABLE_DEBUG_LIGHTING = ENABLE_CLUSTER_DEBUG;

const float MIN_BRIGHTNESS = 1.0 / 256.0;

layout(push_constant) uniform PushConstants {
    uint shadowMapSize;
} pushConstants;

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
    float cascadeDepths[4];
	mat4 cascadeMatrices[4];
    Light lights[LIGHT_COUNT == 0 ? 1 : LIGHT_COUNT];
} lightsData;
layout(set = 0, binding = 8, scalar) readonly restrict buffer Clusters {
    Cluster clusters[512];
} clusters;

layout(set = 0, binding = 6) uniform sampler2DShadow directionalLightShadowMapAtlas;
layout(set = 1, binding = 0) uniform sampler2DShadow shadowMaps[];

layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in float inDistance;
layout(location = 5) flat in uint inTextureIndex;

layout(location = 0) out vec4 outColor;

uint getCascadeIndex(const float distance) {
    for (uint cascadeIndex = 0; cascadeIndex < 4; ++cascadeIndex) {
        if (distance < lightsData.cascadeDepths[cascadeIndex]) {
            return cascadeIndex;
        }
    }
    return 4;
}

uint getClusterIndex(const vec3 position, const float distance) {
	const float cameraDepth = camera.farPlane - camera.nearPlane;
	const vec4 transformedPosition = camera.transformMatrix * vec4(position, 1);
    const vec3 floatingCluster = vec3(transformedPosition.xy / (2 * transformedPosition.w) + 0.5, (distance - camera.nearPlane) / cameraDepth) * 8;
	const uvec3 cluster = uvec3(clamp(floatingCluster, vec3(0), vec3(7.99999)));
	return cluster.x + 8 * cluster.y + 64 * cluster.z;
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

vec2 getSoftShadowKernel(const float sampleIndex) {
    float r = sqrt(sampleIndex + 0.5) / sqrt(float(SAMPLE_COUNT));
    float theta = sampleIndex * 2.4;
    return vec2(cos(theta) * r, sin(theta) * r);
}

float sampleShadowMapInternal(nonuniformEXT sampler2DShadow shadowMap, const vec2 uv, const float depth, const float size) {
    if (SAMPLE_COUNT == 0) {
        return texture(shadowMap, vec3(uv, depth));
    }

    const float r = fract(dot(gl_FragCoord.xy, MAGIC)) * 6.283185307179586;
    const float sr = sin(r);
    const float cr = cos(r);
    const mat2 diskRotation = mat2(vec2(cr, -sr), vec2(sr, cr));

    float sum = 0.0;
    vec2 sampleUv = uv + size * (diskRotation * getSoftShadowKernel(0));
    for (uint i = 0; i < SAMPLE_COUNT; i++) {
        float factor = texture(shadowMap, vec3(sampleUv, depth));
        sampleUv = uv + size * (diskRotation * getSoftShadowKernel(i + 1));
        sum += factor;
    }
    return sum / float(SAMPLE_COUNT);
}

float sampleShadowMap(nonuniformEXT sampler2DShadow shadowMap, const vec2 uv, const float depth) {
    return sampleShadowMapInternal(shadowMap, uv * 0.5 + 0.5, depth, SAMPLE_RADIUS / float(pushConstants.shadowMapSize));
}

float sampleDirectionalShadowMap(const uint cascadeIndex, const vec3 coord) {
    return sampleShadowMapInternal(directionalLightShadowMapAtlas, (coord.xy * 0.25 + 0.25 + vec2(cascadeIndex % 2, cascadeIndex / 2) * 0.5) , coord.z, SAMPLE_RADIUS / float(2 * pushConstants.shadowMapSize));
}

vec3 getLightingColor(const vec3 position, const vec3 normal, const uint cascadeIndex) {
    if (LIGHT_COUNT == 0) {
        return vec3(0);
    }
    vec3 lightingColor = vec3(0);
#ifdef USE_CLUSTERED
    const uint clusterIndex = getClusterIndex(inPosition, inDistance);
    for (uint i = 0; i < clusters.clusters[clusterIndex].lightCount; i++) {
        #define light lightsData.lights[clusters.clusters[clusterIndex].lightIndices[i]]
#else
    for (uint i = 0; i < LIGHT_COUNT; i++) {
        #define light lightsData.lights[i]
#endif
        if (light.type == LIGHT_TYPE_DIRECTIONAL) {
            if (cascadeIndex == 4) {
                lightingColor += light.brightness * max(dot(light.negativeForwardDirection, normal), 0) * light.color;
                continue;
            }
            const vec4 worldPosition = lightsData.cascadeMatrices[cascadeIndex] * vec4(position, 1);
            const vec4 coord = worldPosition / worldPosition.w;
            if (coord.x >= -1 && coord.x <= 1 && coord.y >= -1 && coord.y <= 1) {
                const float factor = sampleDirectionalShadowMap(cascadeIndex, coord.xyz);
                if (factor < EPSILON) {
                    continue;
                }
                lightingColor += factor * light.brightness * max(dot(light.negativeForwardDirection, normal), 0) * light.color;
            }
        } else {
            const vec3 lightToWorld = light.position - position;
            const float distance = length(lightToWorld);
            if (distance > light.maxDistance) {
                continue;
            }
            const vec3 lightToWorldNormalized = normalize(lightToWorld);
            if (light.type == LIGHT_TYPE_SPOT) {
                const float dottedDirection = dot(lightToWorldNormalized, light.negativeForwardDirection);
                if (dottedDirection < 0) {
                    continue;
                }
                const float normalFactor = dot(lightToWorldNormalized, normal);
                if (normalFactor < EPSILON) {
                    continue;
                }
                const float theta = degrees(acos(dottedDirection));
                if (theta > light.fadingAngle) {
                    continue;
                }
                const vec4 worldPosition = light.transformMatrix * vec4(position, 1);
                const vec4 coord = worldPosition / (worldPosition.w - 0.01);
                if (coord.x >= -1 && coord.x <= 1 && coord.y >= -1 && coord.y <= 1) {
                    const float brightness = getLightBrightness(light, distance, theta);
                    if (brightness < MIN_BRIGHTNESS) {
                        continue;
                    }
                    const float factor = sampleShadowMap(shadowMaps[nonuniformEXT(light.shadowMapIndex)], coord.xy, coord.z);
                    if (factor < EPSILON) {
                        continue;
                    }
                    if (light.cookieTextureIndex != 0) {
                        const vec3 cookieColor = texture(textureSampler[nonuniformEXT(light.cookieTextureIndex - 1)], coord.xy * 0.5 + 0.5).rgb;
                        lightingColor += factor * brightness * normalFactor * light.color * cookieColor;
                    } else {
                        lightingColor += factor * brightness * normalFactor * light.color;
                    }
                }
            } else {
                const float normalFactor = dot(lightToWorldNormalized, normal);
                if (normalFactor < EPSILON) {
                    continue;
                }
                const float brightness = getLightBrightness(light, distance, 0);
                if (brightness < MIN_BRIGHTNESS) {
                    continue;
                }
                const vec3 lightToWorldAbs = abs(lightToWorld);
                const float scale = max(max(lightToWorldAbs.x, lightToWorldAbs.y), lightToWorldAbs.z);
                const float comparisonDepth = light.transformMatrix[2][2] + light.transformMatrix[3][2] / (scale - 0.01);
                float factor;
                if (scale == lightToWorldAbs.x) {
                    if (scale == lightToWorld.x) {
                        factor = sampleShadowMap(shadowMaps[nonuniformEXT(light.shadowMapIndex)], lightToWorld.zy / -scale, comparisonDepth);
                    } else {
                        factor = sampleShadowMap(shadowMaps[nonuniformEXT(light.shadowMapIndex) + 1], vec2(lightToWorld.z, -lightToWorld.y) / scale, comparisonDepth);
                    }
                } else if (scale == lightToWorldAbs.y) {
                    if (scale == lightToWorld.y) {
                        factor = sampleShadowMap(shadowMaps[nonuniformEXT(light.shadowMapIndex) + 2], lightToWorld.xz / scale, comparisonDepth);
                    } else {
                        factor = sampleShadowMap(shadowMaps[nonuniformEXT(light.shadowMapIndex) + 3], vec2(lightToWorld.x, -lightToWorld.z) / scale, comparisonDepth);
                    }
                } else {
                    if (scale == lightToWorld.z) {
                        factor = sampleShadowMap(shadowMaps[nonuniformEXT(light.shadowMapIndex) + 4], vec2(lightToWorld.x, -lightToWorld.y) / scale, comparisonDepth);
                    } else {
                        factor = sampleShadowMap(shadowMaps[nonuniformEXT(light.shadowMapIndex) + 5], lightToWorld.xy / -scale, comparisonDepth);
                    }
                }
                if (factor < EPSILON) {
                    continue;
                }
                lightingColor += factor * brightness * normalFactor * light.color;
            }
        }
    }
    return lightingColor;
}

void debugLighting() {
    outColor = vec4(0, 0, 0, 1);
    if (ENABLE_CLUSTER_DEBUG) {
        outColor.r = float(clusters.clusters[getClusterIndex(inPosition, inDistance)].lightCount) / float(LIGHT_COUNT);
		vec3 lightingColor = getLightingColor(inPosition, normalize(inNormal), getCascadeIndex(inDistance));
		outColor.rgb += vec3((lightingColor.x + lightingColor.y + lightingColor.z) / 12);
        return;
	}
}
