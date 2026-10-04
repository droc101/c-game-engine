//
// Created by droc101 on 10/4/26.
//

#ifndef GAME_LOGICCAST_H
#define GAME_LOGICCAST_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition logicCastActorDefinition;

#define LOGIC_CAST_ACTOR_NAME "logic_cast"

#define LOGIC_CAST_INPUT_CAST "cast"

#define LOGIC_CAST_OUTPUT_ON_CAST_BYTE "on_cast_byte"
#define LOGIC_CAST_OUTPUT_ON_CAST_INT "on_cast_int"
#define LOGIC_CAST_OUTPUT_ON_CAST_FLOAT "on_cast_float"
#define LOGIC_CAST_OUTPUT_ON_CAST_BOOL "on_cast_bool"
#define LOGIC_CAST_OUTPUT_ON_CAST_SIZE_T "on_cast_size_t"

void RegisterLogicCast();

#endif //GAME_LOGICCAST_H
