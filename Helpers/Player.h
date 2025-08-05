//
// Created by NBT22 on 7/31/25.
//

#ifndef PLAYER_H
#define PLAYER_H

#include "../defines.h"

void PlayerContactListenerInit();

void PlayerContactListenerDestroy();

void CreatePlayerCollider(Level *level);

void MovePlayer(const Player *player, float *distanceTraveled);

#endif //PLAYER_H
