//
// Created by droc101 on 6/28/26.
//

#ifndef GAME_TRIGGERMAP_H
#define GAME_TRIGGERMAP_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition triggerMapActorDefinition;

#define TRIGGER_MAP_ACTOR_NAME "trigger_map"

#define TRIGGER_MAP_INPUT_FORCE_TRIGGER "force_trigger"
#define TRIGGER_MAP_INPUT_ENABLE "enable"
#define TRIGGER_MAP_INPUT_DISABLE "disable"

void RegisterTriggerMap();

#endif //GAME_TRIGGERMAP_H
