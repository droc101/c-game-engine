//
// Created by droc101 on 11/7/2024.
//

#ifndef DOOR_H
#define DOOR_H

#include <joltc/Math/Transform.h>
#include "../Helpers/Core/KVList.h"
#include "../Structs/Actor.h"

void DoorInit(Actor *this, const KvList *params, Transform *transform);

#endif //DOOR_H
