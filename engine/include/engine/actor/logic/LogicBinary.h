//
// Created by droc101 on 6/19/25.
//

#ifndef LOGICBINARY_H
#define LOGICBINARY_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition logicBinaryActorDefinition;

#define LOGIC_BINARY_ACTOR_NAME "logic_binary"

#define LOGIC_BINARY_INPUT_OPERAND_A "operand_a"
#define LOGIC_BINARY_INPUT_OPERAND_B "operand_b"
#define LOGIC_BINARY_INPUT_EXECUTE "execute"

#define LOGIC_BINARY_OUTPUT_ON_TRUE "on_true"
#define LOGIC_BINARY_OUTPUT_ON_FALSE "on_false"
#define LOGIC_BINARY_OUTPUT_EXECUTION_RESULT "execution_result"

void RegisterLogicBinary();

#endif //LOGICBINARY_H
