//
// Created by droc101 on 4/9/26.
//

#ifndef GAME_LIGHT_H
#define GAME_LIGHT_H

#include <cglm/vec3.h>

typedef struct PointLight PointLight;

struct PointLight
{
	/// The world space position of the light
	vec3 position;
	/// The color of the light
	vec3 color;
	/// The brightness scale of the light
	float brightnessScale;
	/// The maximum range of the light
	float range;
	/// The attenuation of the light
	float attenuation;
};

#endif //GAME_LIGHT_H
