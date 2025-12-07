//
// Created by NBT22 on 7/31/25.
//

#include <engine/debug/DPrint.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/Physics.h>
#include <engine/physics/PlayerPhysics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Map.h>
#include <engine/structs/Player.h>
#include <engine/subsystem/Input.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/RVec3.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/Body.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/PhysicsMaterial.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <joltc/Physics/Collision/ShapeFilter.h>
#include <joltc/types.h>
#include <math.h>
#include <SDL_gamecontroller.h>
#include <SDL_mouse.h>
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void CreatePlayer(Player *player, JPH_PhysicsSystem *physicsSystem)
{
	player->transform.rotation = JPH_Quat_Identity;
	player->canDropHeldActor = true;
	player->isNoclipActive = true; // TODO remove once physics work

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
