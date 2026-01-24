//
// Created by droc101 on 1/16/26.
//

#ifndef GAME_GLOBALFOG_H
#define GAME_GLOBALFOG_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition globalFogActorDefinition;

#define GLOBAL_FOG_ACTOR_NAME "global_fog"

#define GLOBAL_FOG_INPUT_SET "set"
#define GLOBAL_FOG_INPUT_SET_INSTANT "set_instant"

void RegisterGlobalFog();

#endif //GAME_GLOBALFOG_H
