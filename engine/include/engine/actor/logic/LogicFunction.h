//
// Created by droc101 on 10/4/26.
//

#ifndef GAME_LOGICFUNCTION_H
#define GAME_LOGICFUNCTION_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition logicFunctionActorDefinition;

#define LOGIC_FUNCTION_ACTOR_NAME "logic_function"

#define LOGIC_FUNCTION_INPUT_ENABLE "enable"
#define LOGIC_FUNCTION_INPUT_DISABLE "disable"
#define LOGIC_FUNCTION_INPUT_RUN "run"

#define LOGIC_FUNCTION_OUTPUT_ON_RUN "on_run"

void RegisterLogicFunction();

#endif //GAME_LOGICFUNCTION_H
