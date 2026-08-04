#version 460

#extension GL_EXT_scalar_block_layout: require

layout(binding = 2, scalar) uniform TransformMatrixBuffer {
	mat4 matrix;
} transform;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec2 inLightmapUV;
layout(location = 3) in uint inTextureIndex;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec2 outLightmapUV;
layout(location = 2) flat out uint outTextureIndex;

void main() {
    outUV = inUV;
    outLightmapUV = inLightmapUV;
    outTextureIndex = inTextureIndex;
    gl_Position = transform.matrix * vec4(inPosition, 1);
}




