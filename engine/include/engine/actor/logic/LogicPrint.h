//
// Created by droc101 on 10/4/26.
//

#ifndef GAME_LOGICPRINT_H
#define GAME_LOGICPRINT_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition logicPrintActorDefinition;

#define LOGIC_PRINT_ACTOR_NAME "logic_print"

#define LOGIC_PRINT_INPUT_PRINT_VALUE "print_value"

void RegisterLogicPrint();

#endif //GAME_LOGICPRINT_H
