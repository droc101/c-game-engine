//
// Created by droc101 on 10/4/26.
//

#ifndef GAME_LOGICTIMER_H
#define GAME_LOGICTIMER_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition logicTimerActorDefinition;

#define LOGIC_TIMER_ACTOR_NAME "logic_timer"

#define LOGIC_TIMER_INPUT_START "start"
#define LOGIC_TIMER_INPUT_STOP "stop"
#define LOGIC_TIMER_INPUT_FIRE "fire"

#define LOGIC_TIMER_OUTPUT_ON_TIMER_EXPIRED "on_timer_expired"

void RegisterLogicTimer();

#endif //GAME_LOGICTIMER_H
