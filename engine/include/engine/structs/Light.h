//
// Created by droc101 on 4/9/26.
//

#ifndef GAME_LIGHT_H
#define GAME_LIGHT_H

#include <cglm/vec3.h>
#include <stdint.h>

typedef enum LightType LightType;

typedef struct Light Light;

enum LightType
{
	LIGHT_TYPE_POINT,
	LIGHT_TYPE_SPOT,
	LIGHT_TYPE_AREA,
	LIGHT_TYPE_DIRECTIONAL,
};

struct Light
{
	LightType type;
	/// The world space transform of the light
	Transform transform;
	Vector3 negativeForwardDirection;
	/// The color of the light
	vec3 color;
	/// The brightness scale of the light
	float brightness;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
	float attenuationMultiplier;
	float brightAngle;
	float fadingAngle;
	uint32_t shadowMapIndex;
	float _padding[2];
	CGLM_ALIGN_MAT mat4 transformMatrix;
};

#endif //GAME_LIGHT_H
