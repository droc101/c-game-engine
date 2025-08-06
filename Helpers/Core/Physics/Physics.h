//
// Created by noah on 2/10/25.
//

#ifndef INIT_H
#define INIT_H

#include "../../../defines.h"

void PhysicsInitGlobal(GlobalState *state);

void PhysicsDestroyGlobal(const GlobalState *state);

void PhysicsInitLevel(Level *level);

void PhysicsDestroyLevel(const Level *level, JPH_BodyInterface *bodyInterface);

#endif //INIT_H
