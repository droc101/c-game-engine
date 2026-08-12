#version 460

#extension GL_EXT_scalar_block_layout : require

layout(binding = 2, scalar) uniform CameraBuffer {
	mat4 transformMatrix;
	mat4 viewMatrix;
	vec3 position;
} camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in mat4 inTransformMatrix;
layout(location = 8) in vec4 inModColor;
layout(location = 9) in vec4 inMaterialColor;
layout(location = 10) in uint inTextureIndex;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out float outDistance;
layout(location = 5) flat out uint outTextureIndex;

void main() {
	outPosition = inTransformMatrix * vec4(inPosition, 1);
    outColor = inColor * inMaterialColor * inModColor;
    outUV = inUV;
    outNormal = (inTransformMatrix * vec4(inNormal, 0)).xyz;
    outDistance = (camera.viewMatrix * outPosition).z;
    outTextureIndex = inTextureIndex;
    gl_Position = camera.transformMatrix * outPosition;
}
