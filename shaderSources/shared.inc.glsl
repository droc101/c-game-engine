// Include only. This file will not compile as a standalone module.

#extension GL_EXT_debug_printf : require
#extension GL_EXT_buffer_reference : require
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

struct CullingInfo {
    vec3 position;
    float radius;
    uint castsShadows;
};

struct ActorModelCullingInfo {
    vec3 position;
    float radius;
    uint castsShadows;
    uint drawInfoIndex;
};

struct ModelInstanceData
{
	mat4 transformMatrix;
	vec4 materialColor;
	uint textureIndex;
};

struct ActorModelInstanceData
{
	mat4 transformMatrix;
	vec4 modColor;
	vec4 materialColor;
	uint textureIndex;
    float _padding[7];
};

struct ActorWallInstanceData
{
	vec3 position;
	vec2 scale;
	vec2 axis;
	vec2 centerOffset;
	vec4 rotationQuat;
	uint textureIndex;
	vec2 uvScale;
	vec2 uvOffset;
	vec4 modColor;
};

struct VkDrawIndexedIndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;
};

struct DrawActorModelIndexedIndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;

    uint instanceIndex;
};


layout(scalar, buffer_reference) readonly buffer CullingInfoBuffer {
    uint count;
    CullingInfo cullingInfos[];
};

layout(scalar, buffer_reference) readonly buffer ActorModelCullingInfoBuffer {
    uint count;
    ActorModelCullingInfo cullingInfos[];
};

layout(scalar, buffer_reference) readonly buffer MapModelsUnculledDrawInfoBuffer {
    VkDrawIndexedIndirectCommand drawInfos[];
};

layout(scalar, buffer_reference) writeonly buffer InstanceIndicesBuffer {
    uint indices[];
};

layout(scalar, buffer_reference) buffer MapModelsOutputDrawInfoBuffer {
    uint count;
    VkDrawIndexedIndirectCommand drawInfos[];
};

layout(scalar, buffer_reference) buffer ActorWallsDrawInfoBuffer {
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

layout(scalar, buffer_reference) buffer ActorModelsDrawInfoBuffer {
    DrawActorModelIndexedIndirectCommand drawInfos[];
};

struct FrustumCullingData
{
    Frustum frustum;


    CullingInfoBuffer shadedOpaqueMapModelsCullingInfo;
    MapModelsUnculledDrawInfoBuffer shadedOpaqueMapModelsUnculledDrawInfo;
    MapModelsOutputDrawInfoBuffer shadedOpaqueMapModelsOutputDrawInfo;

    CullingInfoBuffer unshadedOpaqueMapModelsCullingInfo;
    MapModelsUnculledDrawInfoBuffer unshadedOpaqueMapModelsUnculledDrawInfo;
    MapModelsOutputDrawInfoBuffer unshadedOpaqueMapModelsOutputDrawInfo;

    CullingInfoBuffer shadedMapModelsCullingInfo;
    MapModelsUnculledDrawInfoBuffer shadedMapModelsUnculledDrawInfo;
    MapModelsOutputDrawInfoBuffer shadedMapModelsOutputDrawInfo;

    CullingInfoBuffer unshadedMapModelsCullingInfo;
    MapModelsUnculledDrawInfoBuffer unshadedMapModelsUnculledDrawInfo;
    MapModelsOutputDrawInfoBuffer unshadedMapModelsOutputDrawInfo;

    CullingInfoBuffer shadedActorWallsCullingInfo;
    InstanceIndicesBuffer shadedActorWallsInstanceIndices;
    ActorWallsDrawInfoBuffer shadedActorWallsDrawInfo;

    CullingInfoBuffer unshadedActorWallsCullingInfo;
    InstanceIndicesBuffer unshadedActorWallsInstanceIndices;
    ActorWallsDrawInfoBuffer unshadedActorWallsDrawInfo;

    ActorModelCullingInfoBuffer shadedActorModelsCullingInfo;
    InstanceIndicesBuffer shadedActorModelsInstanceIndices;
    ActorModelsDrawInfoBuffer shadedActorModelsDrawInfo;

    ActorModelCullingInfoBuffer unshadedActorModelsCullingInfo;
    InstanceIndicesBuffer unshadedActorModelsInstanceIndices;
    ActorModelsDrawInfoBuffer unshadedActorModelsDrawInfo;


    float _padding[2];
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
