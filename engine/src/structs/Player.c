//
// Created by NBT22 on 7/31/25.
//

#include <engine/debug/DPrint.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/PlayerPhysics.h>
#include <engine/structs/Color.h>
#include <engine/structs/Player.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void CreatePlayer(Player *player, JPH_PhysicsSystem *physicsSystem)
{
	player->transform.rotation = JPH_Quat_Identity;
	player->canDropHeldActor = true;

	CreatePlayerPhysics(player, physicsSystem);
}

void DPrintPlayer(const Player *player)
{
	DPrintF("Position: (%.2f, %.2f, %.2f)",
			COLOR_WHITE,
			false,
			player->transform.position.x,
			player->transform.position.y,
			player->transform.position.z);
	Vector3 playerVelocity;
	JPH_CharacterVirtual_GetLinearVelocity(player->joltCharacter, &playerVelocity);
	const float totalVelocity = Vector3_Length(&playerVelocity);
	DPrintF("Velocity: %.2f (%.2f, %.2f, %.2f)",
			COLOR_WHITE,
			false,
			totalVelocity,
			playerVelocity.x,
			playerVelocity.y,
			playerVelocity.z);
	Vector3 eulerAngles;
	Vector3 axis;
	float angle = NAN;
	JPH_Quat_GetEulerAngles(&player->transform.rotation, &eulerAngles);
	JPH_Quat_GetAxisAngle(&player->transform.rotation, &axis, &angle);
	DPrintF("Rotation: (%.4f, %.4f) (%.2fdeg, %.2fdeg)\nRotation Axis: (%.4f, %.4f, %.4f)",
			COLOR_WHITE,
			false,
			player->transform.rotation.x,
			fabsf(player->transform.rotation.y),
			radToDeg(player->transform.rotation.x),
			radToDeg(fabsf(player->transform.rotation.y)),
			axis.x,
			axis.y,
			axis.z);
	DPrintF("%s Actor: %p", COLOR_WHITE, false, player->hasHeldActor ? "Held" : "Targeted", player->targetedActor);
}
