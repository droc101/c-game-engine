//
// Created by droc101 on 4/9/26.
//

#ifndef GAME_LIGHT_H
#define GAME_LIGHT_H

#include <engine/graphics/std140.h>

typedef struct PointLight PointLight;

struct STD140 PointLight
{
	STD140_VEC3 position;
	STD140_VEC3 color;
	STD140_FLOAT brightnessScale;
	STD140_FLOAT range;
	STD140_FLOAT attenuation;
};

#endif //GAME_LIGHT_H
