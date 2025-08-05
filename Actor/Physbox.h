//
// Created by droc101 on 4/28/25.
//

#ifndef PHYSBOX_H
#define PHYSBOX_H

#include "../defines.h"

void PhysboxInit(Actor *this, const KvList *params, Transform *transform);
void PhysboxUpdate(Actor *this, double delta);

#endif //PHYSBOX_H
