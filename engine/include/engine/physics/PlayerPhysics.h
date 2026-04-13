//
// Created by NBT22 on 10/20/25.
//

#ifndef GAME_PLAYERPHYSICS_H
#define GAME_PLAYERPHYSICS_H

#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Player.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>

/// Player movement speed (in units per second)
extern const float MOVE_SPEED;
/// Player movement speed when ctrl is held
extern const float SLOW_MOVE_SPEED;
/// The maximum angle that the player can walk on, in degrees
extern const float MAX_WALKABLE_SLOPE;

void PlayerPersistentStateInit();

void PlayerPersistentStateDestroy();

void CreatePlayerPhysics(Player *player, JPH_PhysicsSystem *physicsSystem);

void TeleportPlayer(Player *player, const Transform *transform);

void MovePlayer(const Player *player, float *distanceTraveled, double delta, bool allowInput);

void UpdatePlayer(Player *player, const JPH_PhysicsSystem *physicsSystem, float deltaTime, bool allowMovement);

const Color *GetCrosshairColor();

void UpdatePlayerCamera(GlobalState *state);

#endif //GAME_PLAYERPHYSICS_H
