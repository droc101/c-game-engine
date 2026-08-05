#version 460

#extension GL_EXT_scalar_block_layout: require

const mat4 transforms[6] = {
	mat4(
         0,  0,  1,  0,
         0, -1,  0,  0,
         1,  0,  0,  0,
         0,  0,  0,  1
	),
    mat4(
         0,  0, -1,  0,
         0, -1,  0,  0,
        -1,  0,  0,  0,
         0,  0,  0,  1
	),
	mat4(
         1,  0,  0,  0,
         0,  0,  1,  0,
         0, -1,  0,  0,
         0,  0,  0,  1
	),
	mat4(
         1,  0,  0,  0,
         0,  0, -1,  0,
         0,  1,  0,  0,
         0,  0,  0,  1
	),
	mat4(
         1,  0,  0,  0,
         0, -1,  0,  0,
         0,  0, -1,  0,
         0,  0,  0,  1
	),
	mat4(
        -1,  0,  0,  0,
         0, -1,  0,  0,
         0,  0,  1,  0,
         0,  0,  0,  1
	),
};

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
    float _padding[3];
	mat4 transformMatrix;
};

layout(push_constant) uniform PushConstants {
    uint lightIndex;
    uint faceIndex;
} pushConstants;

layout(scalar, set = 0, binding = 5) readonly restrict buffer LightsData {
	uint lightCount;
    Light lights[];
} lightsData;

layout(location = 0) in vec2 inVertexPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inActorPosition;
layout(location = 3) in vec2 inScale;
layout(location = 4) in vec2 inAxis;
layout(location = 5) in vec2 inCenterOffset;
layout(location = 6) in vec4 inRotationQuat;
layout(location = 7) in uint inTextureIndex;
layout(location = 8) in vec2 inUvScale;
layout(location = 9) in vec2 inUvOffset;
layout(location = 10) in vec4 inModColor;

layout(location = 0) out vec4 outUvAlphaDistance;
layout(location = 1) flat out uint outTextureIndex;

vec3 rotateVec3ByQuat(vec3 point, vec4 quat){ 
  return point + 2.0 * cross(quat.xyz, cross(quat.xyz, point) + quat.w * point);
}

vec3 getVec3FromVec2(vec2 vec) {
  return vec3(vec.x * inAxis.x, vec.y, vec.x * inAxis.y);
}

void main() {
    outTextureIndex = inTextureIndex;
    const vec3 worldPosition = rotateVec3ByQuat(getVec3FromVec2(inVertexPosition * inScale + inCenterOffset), inRotationQuat) + inActorPosition;
	if (lightsData.lights[pushConstants.lightIndex].type == LIGHT_TYPE_POINT) {
        const vec3 lightToWorld = worldPosition - lightsData.lights[pushConstants.lightIndex].position;
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * (transforms[pushConstants.faceIndex] * vec4(-lightToWorld.xy, lightToWorld.z, 1));
        outUvAlphaDistance = vec4(inUV * inUvScale * (inScale / vec2(16.0)) + inUvOffset, inModColor.a, length(lightToWorld));
	} else {
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * vec4(worldPosition, 1);
        outUvAlphaDistance = vec4(inUV * inUvScale * (inScale / vec2(16.0)) + inUvOffset, inModColor.a, 0);
	}
}
