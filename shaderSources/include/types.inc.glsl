// Include only. This file will not compile as a standalone module.

#extension GL_EXT_debug_printf : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct Transform {
    vec3 position;
    vec4 rotation;
};

const uint LIGHT_TYPE_POINT = 0u;
const uint LIGHT_TYPE_SPOT = 1u;
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
    float maxDistance;
    float _padding;
	mat4 transformMatrix;
};

struct Frustum {
	mat4 viewMatrix;
    float nearPlane;
    float farPlane;
    float frustumPlanes[4];

    float _padding[6];
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


layout(scalar, buffer_reference, buffer_reference_align = 4) readonly buffer CullingInfoBuffer {
    uint count;
    CullingInfo cullingInfos[];
};

layout(scalar, buffer_reference, buffer_reference_align = 4) readonly buffer ActorModelCullingInfoBuffer {
    uint count;
    ActorModelCullingInfo cullingInfos[];
};

layout(scalar, buffer_reference, buffer_reference_align = 4) readonly buffer DrawIndexedIndirectBuffer {
    VkDrawIndexedIndirectCommand drawInfos[];
};

layout(scalar, buffer_reference, buffer_reference_align = 4) readonly buffer UnculledInstanceIndicesBuffer {
    uint indices[];
};

layout(scalar, buffer_reference, buffer_reference_align = 4) writeonly buffer OutputInstanceIndicesBuffer {
    uint indices[];
};

layout(scalar, buffer_reference, buffer_reference_align = 4) buffer DrawIndexedIndirectCountBuffer {
    uint count;
    VkDrawIndexedIndirectCommand drawInfos[];
};

layout(scalar, buffer_reference, buffer_reference_align = 4) buffer ActorWallsDrawInfoBuffer {
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

struct FrustumCullingData
{
    Frustum frustum;


    CullingInfoBuffer shadedOpaqueMapModelsCullingInfo;
    DrawIndexedIndirectBuffer shadedOpaqueMapModelsUnculledDrawInfo;
    DrawIndexedIndirectCountBuffer shadedOpaqueMapModelsOutputDrawInfo;

    CullingInfoBuffer unshadedOpaqueMapModelsCullingInfo;
    DrawIndexedIndirectBuffer unshadedOpaqueMapModelsUnculledDrawInfo;
    DrawIndexedIndirectCountBuffer unshadedOpaqueMapModelsOutputDrawInfo;

    CullingInfoBuffer shadedMapModelsCullingInfo;
    DrawIndexedIndirectBuffer shadedMapModelsUnculledDrawInfo;
    DrawIndexedIndirectCountBuffer shadedMapModelsOutputDrawInfo;

    CullingInfoBuffer unshadedMapModelsCullingInfo;
    DrawIndexedIndirectBuffer unshadedMapModelsUnculledDrawInfo;
    DrawIndexedIndirectCountBuffer unshadedMapModelsOutputDrawInfo;

    CullingInfoBuffer shadedActorWallsCullingInfo;
    OutputInstanceIndicesBuffer shadedActorWallsInstanceIndices;
    ActorWallsDrawInfoBuffer shadedActorWallsDrawInfo;

    CullingInfoBuffer unshadedActorWallsCullingInfo;
    OutputInstanceIndicesBuffer unshadedActorWallsInstanceIndices;
    ActorWallsDrawInfoBuffer unshadedActorWallsDrawInfo;

    ActorModelCullingInfoBuffer shadedActorModelsCullingInfo;
    UnculledInstanceIndicesBuffer shadedActorModelsUnculledInstanceIndices;
    OutputInstanceIndicesBuffer shadedActorModelsInstanceIndices;
    DrawIndexedIndirectBuffer shadedActorModelsDrawInfo;

    ActorModelCullingInfoBuffer unshadedActorModelsCullingInfo;
    UnculledInstanceIndicesBuffer unshadedActorModelsUnculledInstanceIndices;
    OutputInstanceIndicesBuffer unshadedActorModelsInstanceIndices;
    DrawIndexedIndirectBuffer unshadedActorModelsDrawInfo;
};
