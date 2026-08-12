#version 460

#extension GL_EXT_scalar_block_layout: require

layout(binding = 2, scalar) uniform CameraBuffer {
	mat4 transformMatrix;
	mat4 viewMatrix;
	vec3 position;
} camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec2 inLightmapUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in uint inTextureIndex;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec2 outLightmapUV;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out float outDistance;
layout(location = 5) flat out uint outTextureIndex;

void main() {
	outPosition = inPosition;
    outUV = inUV;
    outLightmapUV = inLightmapUV;
    outNormal = inNormal;
    outDistance = (camera.viewMatrix * vec4(inPosition, 1)).z;
    outTextureIndex = inTextureIndex;
    gl_Position = camera.transformMatrix * vec4(inPosition, 1);
}
