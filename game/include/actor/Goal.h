//
// Created by droc101 on 7/12/2024.
//

#ifndef GAME_GOAL_H
#define GAME_GOAL_H

#include <engine/structs/ActorDefinition.h>

#define ACTOR_TYPE_GOAL (2 + FIRST_GAME_ACTOR_TYPE_ID)

#define GOAL_ACTOR_NAME "prop_goal"

#define GOAL_INPUT_ENABLE "enable"
#define GOAL_INPUT_DISABLE "disable"

#define GOAL_OUTPUT_COLLECTED "collected"

void RegisterGoal();

#endif //GAME_GOAL_H
