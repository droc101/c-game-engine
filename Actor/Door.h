//
// Created by droc101 on 11/7/2024.
//

#ifndef DOOR_H
#define DOOR_H

#include "../Structs/Actor.h"

void DoorInit(Actor *this, const KvList *params, Transform *transform);

void DoorUpdate(Actor *this, double delta);

void DoorDestroy(Actor *this);


#endif //DOOR_H
