#version 460

#extension GL_EXT_scalar_block_layout : require

layout(binding = 2, scalar) uniform TransformMatrixBuffer {
	mat4 matrix;
} transform;

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

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec3 outNormal;
layout(location = 4) flat out uint outTextureIndex;

vec3 rotateVec3ByQuat(vec3 point, vec4 quat){ 
  return point + 2.0 * cross(quat.xyz, cross(quat.xyz, point) + quat.w * point);
}

vec3 getVec3FromVec2(vec2 vec) {
  return vec3(vec.x * inAxis.x, vec.y, vec.x * inAxis.y);
}

void main() {
	outPosition = vec4(rotateVec3ByQuat(getVec3FromVec2(inVertexPosition * inScale + inCenterOffset), inRotationQuat) + inActorPosition, 1);
    outColor = inModColor;
    outUV = inUV * inUvScale * (inScale / vec2(16.0)) + inUvOffset;
    const float normalMultiplier = gl_VertexIndex < 6 ? 1 : -1;
    outNormal = rotateVec3ByQuat(normalMultiplier * vec3(-inAxis.y, 0, inAxis.x), inRotationQuat);
    outTextureIndex = inTextureIndex;
    gl_Position = transform.matrix * outPosition;
}
