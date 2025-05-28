//
// Created by droc101 on 4/28/25.
//

#ifndef LASER_H
#define LASER_H

#include <box2d/box2d.h>
#include "../defines.h"

void LaserInit(Actor *this, b2WorldId worldId, KvList *params);
void LaserUpdate(Actor *this, double delta);
void LaserDestroy(Actor *this);

#endif //LASER_H
