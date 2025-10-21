//
// Created by droc101 on 4/28/25.
//

#ifndef LASER_H
#define LASER_H

#include <engine/structs/ActorDefinition.h>

#define ACTOR_TYPE_LASER (3 + FIRST_GAME_ACTOR_TYPE_ID)

#define LASER_ACTOR_NAME "prop_laser"

#define LASER_INPUT_TURN_ON "turn_on"
#define LASER_INPUT_TURN_OFF "turn_off"

typedef enum LaserHeight
{
	LASER_HEIGHT_FLOOR,
	LASER_HEIGHT_MIDDLE,
	LASER_HEIGHT_CEILING,
	LASER_HEIGHT_TRIPLE
} LaserHeight;

void LaserRaycastFiltersInit();

void LaserRaycastFiltersDestroy();

void RegisterLaser();

#endif //LASER_H
