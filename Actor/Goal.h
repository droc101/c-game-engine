//
// Created by droc101 on 7/12/2024.
//

#ifndef GAME_GOAL_H
#define GAME_GOAL_H

#include <joltc/Math/Transform.h>
#include "../Helpers/Core/KVList.h"
#include "../Structs/Actor.h"

void GoalInit(Actor *this, const KvList params, Transform *transform);

#endif //GAME_GOAL_H
