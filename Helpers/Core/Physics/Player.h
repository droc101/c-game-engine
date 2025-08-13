//
// Created by NBT22 on 7/31/25.
//

#ifndef PLAYER_H
#define PLAYER_H

#include "../../../defines.h"

void PlayerPersistentStateInit();

void PlayerPersistentStateDestroy();

void CreatePlayer(Level *level);

void MovePlayer(const Player *player, float *distanceTraveled);

void UpdatePlayer(Player *player, const JPH_PhysicsSystem *physicsSystem, float deltaTime);

const Color *GetCrosshairColor();

void DPrintPlayer(const Level *level);

#endif //PLAYER_H
