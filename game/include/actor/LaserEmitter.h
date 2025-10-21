//
// Created by droc101 on 6/1/25.
//

#ifndef LASEREMITTER_H
#define LASEREMITTER_H

#include <engine/structs/ActorDefinition.h>

#define ACTOR_TYPE_LASER_EMITTER (4 + FIRST_GAME_ACTOR_TYPE_ID)

#define LASER_EMITTER_ACTOR_NAME "prop_laser_emitter"

#define LASER_EMITTER_INPUT_TURN_ON "turn_on"
#define LASER_EMITTER_INPUT_TURN_OFF "turn_off"

void RegisterLaserEmitter();

#endif //LASEREMITTER_H
