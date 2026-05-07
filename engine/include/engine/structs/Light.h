//
// Created by droc101 on 4/9/26.
//

#ifndef GAME_LIGHT_H
#define GAME_LIGHT_H

#include <engine/graphics/std140.h>

typedef struct PointLight PointLight;

struct STD140 PointLight
{
	/// The world space position of the light
	STD140_VEC3 position;
	/// The color of the light
	STD140_VEC3 color;
	/// The brightness scale of the light
	STD140_FLOAT brightnessScale;
	/// The maximum range of the light
	STD140_FLOAT range;
	/// The attenuation of the light
	STD140_FLOAT attenuation;
};

#endif //GAME_LIGHT_H
