//
// Created by NBT22 on 10/20/25.
//

#ifndef GAME_PLAYERPHYSICS_H
#define GAME_PLAYERPHYSICS_H

#include <engine/structs/Color.h>
#include <engine/structs/Player.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>

/// Player movement speed (in units per second)
static const float MOVE_SPEED = 6.0f;
/// Player movement speed when ctrl is held
static const float SLOW_MOVE_SPEED = 0.6f;

/// The maximum angle that the player can walk on, in degrees
static const float MAX_WALKABLE_SLOPE = 50.0f;

void PlayerPersistentStateInit();

void PlayerPersistentStateDestroy();

void CreatePlayerPhysics(Player *player, JPH_PhysicsSystem *physicsSystem);

void TeleportPlayer(Player *player, const Transform *transform);

void MovePlayer(const Player *player, float *distanceTraveled, double delta);

void UpdatePlayer(Player *player, const JPH_PhysicsSystem *physicsSystem, float deltaTime);

const Color *GetCrosshairColor();

#endif //GAME_PLAYERPHYSICS_H
