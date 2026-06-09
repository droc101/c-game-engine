//
// Created by droc101 on 4/9/26.
//

#ifndef GAME_LIGHT_H
#define GAME_LIGHT_H

#include <cglm/vec3.h>

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
};

#endif //GAME_LIGHT_H
