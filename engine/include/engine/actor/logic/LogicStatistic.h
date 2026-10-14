//
// Created by droc101 on 10/4/26.
//

#ifndef GAME_LOGICSTATISTIC_H
#define GAME_LOGICSTATISTIC_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition logicStatisticActorDefinition;

#define LOGIC_STATISTIC_ACTOR_NAME "logic_statistic"

#define LOGIC_STATISTIC_INPUT_ADD "add"
#define LOGIC_STATISTIC_INPUT_SET "set"

void RegisterLogicStatistic();

#endif //GAME_LOGICSTATISTIC_H
