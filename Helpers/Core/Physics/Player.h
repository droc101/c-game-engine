//
// Created by NBT22 on 7/31/25.
//

#ifndef PLAYER_H
#define PLAYER_H

#include <joltc.h>
#include <stdbool.h>
#include "../../../Structs/Actor.h"
#include "../../../Structs/Color.h"

typedef struct Level Level; // We cannot include Level.h here (cyclic dep.)

typedef struct Player Player;

struct Player
{
	/// The player's 3d transform
	Transform transform;
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
	bool canDropHeldActor;
};

void PlayerPersistentStateInit();

void PlayerPersistentStateDestroy();

void CreatePlayer(Level *level);

void MovePlayer(const Player *player, float *distanceTraveled);

void UpdatePlayer(Player *player, const JPH_PhysicsSystem *physicsSystem, float deltaTime);

const Color *GetCrosshairColor();

void DPrintPlayer(const Level *level);

#endif //PLAYER_H
