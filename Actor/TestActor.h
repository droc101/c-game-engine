//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_TESTACTOR_H
#define GAME_TESTACTOR_H

#include "../defines.h"

void TestActorInit(Actor *this, b2WorldId worldId, const KvList *params);

void TestActorUpdate(Actor *this, double delta);

void TestActorDestroy(Actor *this);

#endif //GAME_TESTACTOR_H
