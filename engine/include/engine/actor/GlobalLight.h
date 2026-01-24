//
// Created by droc101 on 1/4/26.
//

#ifndef GAME_GLOBALLIGHT_H
#define GAME_GLOBALLIGHT_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition globalLightActorDefinition;

#define GLOBAL_LIGHT_ACTOR_NAME "global_light"

#define GLOBAL_LIGHT_INPUT_SET "set"
#define GLOBAL_LIGHT_INPUT_SET_INSTANT "set_instant"

void RegisterGlobalLight();

#endif //GAME_GLOBALLIGHT_H
