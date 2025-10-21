//
// Created by droc101 on 7/22/25.
//

#ifndef LOGICCOUNTER_H
#define LOGICCOUNTER_H

#define ACTOR_TYPE_LOGIC_COUNTER 2

#define LOGIC_COUNTER_ACTOR_NAME "logic_counter"

#define LOGIC_COUNTER_INPUT_INCREMENT "increment"
#define LOGIC_COUNTER_INPUT_DECREMENT "decrement"
#define LOGIC_COUNTER_INPUT_ADD "add"
#define LOGIC_COUNTER_INPUT_SUBTRACT "subtract"

#define LOGIC_COUNTER_OUTPUT_HIT_MAX "hit_max"
#define LOGIC_COUNTER_OUTPUT_HIT_MIN "hit_min"
#define LOGIC_COUNTER_OUTPUT_LEFT_MAX "left_max"
#define LOGIC_COUNTER_OUTPUT_LEFT_MIN "left_min"
#define LOGIC_COUNTER_OUTPUT_COUNTER_CHANGED "counter_changed"

void RegisterLogicCounter();

#endif //LOGICCOUNTER_H
