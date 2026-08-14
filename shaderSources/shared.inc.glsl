// Include only. This file will not compile as a standalone module.

#extension GL_EXT_debug_printf : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

struct Transform {
    vec3 position;
    vec4 rotation;
};

const uint LIGHT_TYPE_POINT = 0u;
const uint LIGHT_TYPE_SPOT = 1u;
const uint LIGHT_TYPE_AREA = 2u;
const uint LIGHT_TYPE_DIRECTIONAL = 3u;

struct Light {
    uint type; // Maps to an enum in C
    Transform transform;
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

struct Frustum {
	mat4 viewMatrix;
    float nearPlane;
    float farPlane;
    float frustumPlanes[4];
};

struct ModelCullInfo {
    vec3 position;
    float radius;
    uint castsShadows;
};

struct VkDrawIndexedIndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance; // TODO: If I can avoid writing this for map it will improve performance
};

const vec2 MAGIC_XY = vec2(0.06711056, 0.00583715);
const float MAGIC_Z = 52.9829189;

layout(binding = 2, scalar) readonly restrict uniform CameraBuffer {
	mat4 transformMatrix;
	mat4 viewMatrix;
	vec3 position;
    float nearPlane;
    float farPlane;
    float frustumPlanes[4];
} camera;