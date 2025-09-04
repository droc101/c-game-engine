//
// Created by droc101 on 4/28/25.
//

#ifndef PHYSBOX_H
#define PHYSBOX_H

#include <joltc/Math/Transform.h>
#include "../Helpers/Core/KVList.h"
#include "../Structs/Actor.h"

void PhysboxInit(Actor *this, const KvList params, Transform *transform);

#endif //PHYSBOX_H
