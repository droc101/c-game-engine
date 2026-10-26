//
// Created by NBT22 on 7/31/25.
//

/**
 * @file Player.h
 * Player definitions & utility functions
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <engine/structs/Actor.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Color.h>
#include <engine/structs/Light.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <SDL3/SDL_mutex.h>
#include <stdbool.h>

/// The crosshair color to use when not looking at anything special
#define CROSSHAIR_COLOR_NORMAL COLOR(0xFFFFCCCC)
/// The crosshair color to use when looking at something interactable
#define CROSSHAIR_COLOR_INTERACTABLE COLOR(0xFF006600)
/// The crosshair color to use when looking at an enemy
#define CROSSHAIR_COLOR_ENEMY COLOR(0xffff0000)
/// Invisible crosshair color
#define CROSSHAIR_COLOR_INVISIBLE COLOR(0x00000000)

typedef struct Map Map;
typedef struct Player Player;

/// The player
struct Player
{
	/// The player's 3d transform.
	/// Position is the position as of the start of the last physics tick, rotation is updated on the render thread
	Transform transform;
	/// The change in player position during the last physics tick
	Vector3 deltaPosition;
	/// The Jolt character. Includes the rigid body as well as other useful abstractions
	JPH_CharacterVirtual *joltCharacter;
	/// Aliasing for targeted vs held actor to improve code clarity by differentiating between targeted and held actor
	union
	{
		/// The currently targeted actor
		Actor *targetedActor;
		/// The currently held actor
		Actor *heldActor;
	};
	/// True if the player is currently holding an actor, false if the player is targeting an actor instead
	bool hasHeldActor;
	/// Whether the player is currently in noclip
	bool isNoclipActive;
	/// Whether the player is currently in freecam
	bool isFreecamActive;
	/// Whether the player's flashlight is currently on
	bool isFlashlightOn;

	/// The player's camera
	Camera playerCamera;
	/// The height of view bobbing
	float viewBobbingHeight;
	/// The player's flashlight
	DynamicLight flashlight;

	/// Mutex used for synchronizing player position for interpolation
	SDL_Mutex *mutex;
};

/// @private
void CreatePlayer(Player *player, JPH_PhysicsSystem *physicsSystem);

/// @private
void DestroyPlayer(Player *player);

#endif //PLAYER_H
