//
// Created by droc101 on 9/24/26.
//

#ifndef GAME_SPOTLIGHT_H
#define GAME_SPOTLIGHT_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition spotLightActorDefinition;

#define SPOT_LIGHT_ACTOR_NAME "light_spot_rt"

#define SPOT_LIGHT_INPUT_TURN_ON "turn_on"
#define SPOT_LIGHT_INPUT_TURN_OFF "turn_off"

void RegisterSpotLight();

#endif //GAME_SPOTLIGHT_H
