#version 460

#include "shadow_maps_shared.inc.glsl"

layout(binding = 7, scalar) readonly restrict buffer InstanceData {
    ActorModelInstanceData instanceDatas[];
} instanceDatas[2];

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;
layout(location = 3) in uint inInstanceIndex;

layout(location = 0, component = 0) out vec2 outUv;
layout(location = 0, component = 3) out float outAlpha;
layout(location = 1) out vec3 outPosition;
layout(location = 2) flat out uint outTextureIndex;

void main() {
    const ActorModelInstanceData instanceData = instanceDatas[1].instanceDatas[inInstanceIndex];
	const vec4 transformedPosition = instanceData.transformMatrix * vec4(inPosition, 1);
    outUv = inUV;
	outAlpha = inColor.a * instanceData.materialColor.a * instanceData.modColor.a;
	outPosition = transformedPosition.xyz;
    outTextureIndex = instanceData.textureIndex;
	if (lightsData.lights[pushConstants.lightIndex].type == LIGHT_TYPE_POINT) {
        const vec3 lightToWorld = outPosition - lightsData.lights[pushConstants.lightIndex].transform.position;
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * (pointLightViewMatrices[pushConstants.faceIndex] * vec4(-lightToWorld.xy, lightToWorld.z, 1));
	} else if (lightsData.lights[pushConstants.lightIndex].type == LIGHT_TYPE_SPOT) {
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * transformedPosition;
	} else {
        gl_Position = lightsData.cascadeMatrices[pushConstants.cascadeIndex] * transformedPosition;
		gl_Position.z = min(gl_Position.z, 1);
    }
}
