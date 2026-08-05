#version 460

#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

const uint LIGHT_TYPE_POINT = 0u;
const uint LIGHT_TYPE_SPOT = 1u;
const uint LIGHT_TYPE_AREA = 2u;
const uint LIGHT_TYPE_DIRECTIONAL = 3u;

struct Light {
    uint type; // Maps to an enum in C
    vec3 position;
    vec4 rotation;
    vec3 negativeForwardDirection;
    vec3 color;
    float brightness;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    float attenuationMultiplier;
    float brightAngle; // 0-90 degrees
    float fadingAngle;
    uint shadowMapIndex;
    float _padding[2];
	mat4 transformMatrix;
};

const float MIN_BRIGHTNESS = 1.0 / 256.0;
const vec2 MAGIC_XY = vec2(0.06711056, 0.00583715);
const float MAGIC_Z = 52.9829189;

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

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) flat in uint inTextureIndex;

layout(location = 0) out vec4 outColor;

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
            const vec4 position = light.transformMatrix * inPosition;
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
            const vec3 lightToWorld = light.position - inPosition.xyz;
            const float depth = length(lightToWorld);
            const float factor = texture(pointLightShadowMaps[light.shadowMapIndex], vec4(lightToWorld, depth));
            if (factor < 1e-6) {
                continue;
            }
            lightingColor += factor * getLightColor(light, depth, 0) * max(dot(normalize(lightToWorld), normalize(inNormal)), 0);
        }
    }

    outColor = texture(textureSampler[nonuniformEXT(inTextureIndex)], inUV);
    float fade = clamp(outColor.a * inColor.a, 0.0, 1.0);
    if (fade < 0.001 || fade < fract(MAGIC_Z * fract(dot(gl_FragCoord.xy, MAGIC_XY)))) {
        discard;
    }
	outColor.a = 1.0;
	float fogFactor = clamp((gl_FragCoord.z / gl_FragCoord.w - fog.start) / (fog.end - fog.start), 0.0, 1.0) * fog.colorAlpha;
	outColor.rgb = mix(outColor.rgb * inColor.rgb * globalLighting.color.rgb * lightingColor, fog.color, fogFactor);
	outColor.rgb = clamp(outColor.rgb * globalLighting.exposure, 0.0, 1.0);
}
