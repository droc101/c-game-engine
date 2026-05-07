//
// Created by droc101 on 7/22/25.
//

#ifndef LOGICDECIMAL_H
#define LOGICDECIMAL_H

#include <engine/structs/ActorDefinition.h>

extern ActorDefinition logicDecimalActorDefinition;

#define LOGIC_DECIMAL_ACTOR_NAME "logic_decimal"

#define LOGIC_DECIMAL_INPUT_OPERAND_A "operand_a"
#define LOGIC_DECIMAL_INPUT_OPERAND_B "operand_b"
#define LOGIC_DECIMAL_INPUT_EXECUTE "execute"

#define LOGIC_DECIMAL_OUTPUT_ON_TRUE "on_true"
#define LOGIC_DECIMAL_OUTPUT_ON_FALSE "on_false"
#define LOGIC_DECIMAL_OUTPUT_EXECUTION_RESULT "execution_result"


void RegisterLogicDecimal();

#endif //LOGICDECIMAL_H
