//
// Created by droc101 on 6/1/25.
//

#ifndef LASEREMITTER_H
#define LASEREMITTER_H

#include "../defines.h"

void LaserEmitterInit(Actor *this, const KvList *params, JPH_BodyInterface *bodyInterface);
void LaserEmitterUpdate(Actor *this, double);
void LaserEmitterDestroy(Actor *this);

#endif //LASEREMITTER_H
