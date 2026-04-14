//
// Created by NBT22 on 7/31/25.
//

#include <engine/debug/DPrint.h>
#include <engine/physics/PlayerPhysics.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Player.h>
#include <engine/subsystem/threads/PhysicsThread.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <stdbool.h>
#include <stddef.h>

void CreatePlayer(Player *player, JPH_PhysicsSystem *physicsSystem)
{
	player->transform.rotation = JPH_Quat_Identity;
	player->playerCamera.fov = GetState()->options.fov;
	player->playerCamera.transform.rotation = JPH_Quat_Identity;

	CreatePlayerPhysics(player, physicsSystem);
}

void DPrintPlayer(const Player *player)
{
#ifdef ENABLE_DEBUG_PRINT
	DPrintF("Position: (%.2f, %.2f, %.2f)",
			false,
			COLOR_WHITE,
			player->transform.position.x,
			player->transform.position.y,
			player->transform.position.z);
	Vector3 playerVelocity;
	JPH_CharacterVirtual_GetLinearVelocity(player->joltCharacter, &playerVelocity);
	const float totalVelocity = Vector3_Length(&playerVelocity);
	DPrintF("Velocity: %.2f (%.2f, %.2f, %.2f)",
			false,
			COLOR_WHITE,
			totalVelocity,
			playerVelocity.x,
			playerVelocity.y,
			playerVelocity.z);
	PhysicsThreadLockTickMutex();
	DPrintF("%s Actor: %s %p",
			false,
			COLOR_WHITE,
			player->hasHeldActor ? "Held" : "Targeted",
			player->targetedActor ? player->targetedActor->definition->className : "None",
			player->targetedActor);
	PhysicsThreadUnlockTickMutex();
#endif
}
