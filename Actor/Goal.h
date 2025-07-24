//
// Created by droc101 on 7/12/2024.
//

#ifndef GAME_GOAL_H
#define GAME_GOAL_H

#include "../defines.h"

void GoalInit(Actor *this, const KvList *params, JPH_BodyInterface *bodyInterface);

void GoalUpdate(Actor *this, double /*delta*/);

void GoalDestroy(Actor *this);

#endif //GAME_GOAL_H
