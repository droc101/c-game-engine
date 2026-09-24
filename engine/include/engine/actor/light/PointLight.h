//
// Created by droc101 on 9/24/26.
//

#ifndef GAME_POINTLIGHT_H
#define GAME_POINTLIGHT_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition pointLightActorDefinition;

#define POINT_LIGHT_ACTOR_NAME "light_point_rt"

#define POINT_LIGHT_INPUT_TURN_ON "turn_on"
#define POINT_LIGHT_INPUT_TURN_OFF "turn_off"

void RegisterPointLight();

#endif //GAME_POINTLIGHT_H
