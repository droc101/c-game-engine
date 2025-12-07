//
// Created by NBT22 on 7/31/25.
//

#ifndef PLAYER_H
#define PLAYER_H

#include <engine/structs/Actor.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>
#include <stdbool.h>

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
	bool isNoclipActive;
};

void CreatePlayer(Player *player, JPH_PhysicsSystem *physicsSystem);

void DPrintPlayer(const Player *player);

#endif //PLAYER_H
