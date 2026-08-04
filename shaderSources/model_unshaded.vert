#version 460

#extension GL_EXT_scalar_block_layout : require

layout(binding = 2, scalar) uniform TransformMatrixBuffer {
	mat4 matrix;
} transform;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;
layout(location = 3) in mat4 inTransformMatrix;
layout(location = 7) in vec4 inMaterialColor;
layout(location = 8) in uint inTextureIndex;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) flat out uint outTextureIndex;

void main() {
    outColor = inColor * inMaterialColor;
    outUV = inUV;
    outTextureIndex = inTextureIndex;
    gl_Position = transform.matrix * inTransformMatrix * vec4(inPosition, 1);
}



