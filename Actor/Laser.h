//
// Created by droc101 on 4/28/25.
//

#ifndef LASER_H
#define LASER_H

#include <box2d/box2d.h>
#include "../defines.h"

#define LASER_INPUT_DISABLE 1
#define LASER_INPUT_ENABLE 2

typedef enum LaserHeight
{
	LASER_HEIGHT_FLOOR,
	LASER_HEIGHT_MIDDLE,
	LASER_HEIGHT_CEILING,
	LASER_HEIGHT_TRIPLE
} LaserHeight;

void LaserInit(Actor *this, b2WorldId worldId, const KvList *params);
void LaserUpdate(Actor *this, double delta);
void LaserDestroy(Actor *this);

#endif //LASER_H
