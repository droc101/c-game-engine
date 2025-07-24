//
// Created by droc101 on 4/28/25.
//

#ifndef PHYSBOX_H
#define PHYSBOX_H

#include "../defines.h"

void PhysboxInit(Actor *this, const KvList *params, JPH_BodyInterface *bodyInterface);
void PhysboxUpdate(Actor *this, double delta);
void PhysboxDestroy(Actor *this);

#endif //PHYSBOX_H
