//
// Created by NBT22 on 7/31/25.
//

#ifndef PLAYER_H
#define PLAYER_H

#include "../../../defines.h"

void PlayerPersistentStateInit();

void PlayerPersistentStateDestroy();

void CreatePlayerCollider(Level *level);

void MovePlayer(const Player *player, float *distanceTraveled);

void Update(const Player *player, const JPH_PhysicsSystem *physicsSystem, float deltaTime);

#endif //PLAYER_H
