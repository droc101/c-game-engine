#version 460

#include "../../include/shadow_maps_shared.inc.glsl"

layout(binding = 7, scalar) readonly restrict buffer InstanceData {
    ActorModelInstanceData instanceDatas[];
} instanceDatas[2];

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;
layout(location = 3) in uint inInstanceIndex;

layout(location = 0, component = 0) out vec2 outUv;
layout(location = 0, component = 3) out float outAlpha;
layout(location = 1) flat out uint outTextureIndex;

void main() {
    const ActorModelInstanceData instanceData = instanceDatas[1].instanceDatas[inInstanceIndex];
    outUv = inUV;
	outAlpha = inColor.a * instanceData.materialColor.a * instanceData.modColor.a;
    outTextureIndex = instanceData.textureIndex;
	const vec4 transformedPosition = instanceData.transformMatrix * vec4(inPosition, 1);
	if (pushConstants.lightType == LIGHT_TYPE_POINT) {
        const vec3 lightToWorld = transformedPosition.xyz - lightsData.lights[pushConstants.lightIndex].transform.position;
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * (pointLightViewMatrices[pushConstants.faceIndex] * vec4(lightToWorld, 1));
	} else if (pushConstants.lightType == LIGHT_TYPE_SPOT) {
	    gl_Position = lightsData.lights[pushConstants.lightIndex].transformMatrix * transformedPosition;
	} else if (pushConstants.lightType == LIGHT_TYPE_DIRECTIONAL) {
        gl_Position = lightsData.cascadeMatrices[pushConstants.cascadeIndex] * transformedPosition;
		gl_Position.z = min(gl_Position.z, 1);
    } else {
		gl_Position = camera.transformMatrix * transformedPosition;
	}
}
