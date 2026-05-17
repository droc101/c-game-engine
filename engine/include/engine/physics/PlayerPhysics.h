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

/**
 * Initialize the objects used by the player that will persist for the program's lifetime
 * Currently used for the jolt impl structs specifying callbacks for filters and such
 */
void PlayerPersistentStateInit();

/**
 * Destroy the persistent objects
 */
void PlayerPersistentStateDestroy();

/**
 * Create the physics objects for the player in a given map
 * @param map The map to create the player for
 */
void CreatePlayerPhysics(Map *map);

/**
 * Set a player's transform to an arbitrary value
 * @param player The player to set the transform for
 * @param transform The position and rotation to apply to the player
 */
void SetPlayerTransform(Player *player, const Transform *transform);

/**
 * Update the player's velocity and apply gravity
 * @param player The player to move
 * @param delta The percent of PHYSICS_TARGET_MS that the previous tick took
 * @param allowInput If the player should be allowed to receive inputs
 */
void MovePlayer(const Player *player, double delta, bool allowInput);

/**
 * Update the player's freecam, noclip, and physics state
 * @param player The player to update
 * @param physicsSystem The physics system within which the player resides
 * @param deltaTime The number of milliseconds that the previous tick took
 * @param allowInput If the player should be allowed to receive inputs
 */
void UpdatePlayer(Player *player, const JPH_PhysicsSystem *physicsSystem, float deltaTime, bool allowInput);

/**
 * Get the requested crosshair color
 * @return Crosshair color
 */
const Color *GetCrosshairColor();

/**
 * Update the player's camera, applying any inputs
 * @param state GlobalState pointer
 * @param delta Delta time
 */
void UpdatePlayerCamera(GlobalState *state, double delta);

#endif //GAME_PLAYERPHYSICS_H
