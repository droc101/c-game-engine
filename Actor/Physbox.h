//
// Created by droc101 on 4/28/25.
//

#ifndef PHYSBOX_H
#define PHYSBOX_H

#include "../defines.h"
#include "box2d/id.h"

void PhysboxInit(Actor *this, b2WorldId worldId, const KvList *params);
void PhysboxUpdate(Actor *this, double delta);
void PhysboxDestroy(Actor *this);

#endif //PHYSBOX_H
