//
// Created by NBT22 on 7/31/25.
//

#include "Player.h"
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
#include "../../../config.h"
#include "../../../Debug/DPrint.h"
#include "../../../Structs/Actor.h"
#include "../../../Structs/Color.h"
#include "../../../Structs/GlobalState.h"
#include "../../../Structs/Level.h"
#include "../Input.h"
#include "../MathEx.h"
#include "Physics.h"

static const float actorRaycastMaxDistance = 10.0f;
static const float offset = 1.0f;
/// The code for holding objects is heavily based on the implementation in Kill Spud Simulator/Hide and Seek
/// This variable comes from there, and interestingly enough lower numbers seem to be "smoother"
static const float smoothFactor = 17.5f;
static const float heldActorMaxDistanceSquared = 6.0f;

static Color crosshairColor = COLOR(0xFFFFCCCC);


static bool ActorRaycastBroadPhaseLayerShouldCollide(const JPH_BroadPhaseLayer layer)
{
	switch (layer)
	{
		case BROAD_PHASE_LAYER_STATIC:
		case BROAD_PHASE_LAYER_DYNAMIC:
			return true;
		default:
			return false;
	}
}

static bool ActorRaycastObjectLayerShouldCollide(const JPH_ObjectLayer layer)
{
	switch (layer)
	{
		case OBJECT_LAYER_STATIC:
		case OBJECT_LAYER_DYNAMIC:
			return true;
		default:
			return false;
	}
}

static const JPH_BroadPhaseLayerFilter_Impl actorRaycastBroadPhaseLayerFilterImpl = {
	.ShouldCollide = ActorRaycastBroadPhaseLayerShouldCollide,
};
static const JPH_ObjectLayerFilter_Impl actorRaycastObjectLayerFilterImpl = {
	.ShouldCollide = ActorRaycastObjectLayerShouldCollide,
};
static JPH_BroadPhaseLayerFilter *actorRaycastBroadPhaseLayerFilter;
static JPH_ObjectLayerFilter *actorRaycastObjectLayerFilter;


static void OnContactAdded(const JPH_CharacterVirtual *character,
						   const JPH_BodyId bodyId,
						   JPH_SubShapeId /*subShapeId*/,
						   const JPH_RVec3 * /*contactPosition*/,
						   const Vector3 * /*contactNormal*/,
						   JPH_CharacterContactSettings *ioSettings)
{
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(GetState()->level->physicsSystem);
	Actor *actor = (Actor *)JPH_BodyInterface_GetUserData(bodyInterface, bodyId);
	if (actor == NULL)
	{
		return; // Walls and floor
	}
	Player *player = (Player *)JPH_CharacterVirtual_GetUserData(character);
	ioSettings->canPushCharacter = (actor->actorFlags & ACTOR_FLAG_CAN_PUSH_PLAYER) == ACTOR_FLAG_CAN_PUSH_PLAYER;
	if (player->hasHeldActor && player->heldActor->bodyId == bodyId)
	{
		player->canDropHeldActor = false;
		ioSettings->canPushCharacter = false;
		ioSettings->canReceiveImpulses = false;
	}
	actor->definition->OnPlayerContactAdded(actor, bodyId);
}

static void OnContactPersisted(const JPH_CharacterVirtual *character,
							   const JPH_BodyId bodyId,
							   JPH_SubShapeId /*subShapeId*/,
							   const JPH_RVec3 * /*contactPosition*/,
							   const Vector3 * /*contactNormal*/,
							   JPH_CharacterContactSettings *ioSettings)
{
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(GetState()->level->physicsSystem);
	Actor *actor = (Actor *)JPH_BodyInterface_GetUserData(bodyInterface, bodyId);
	if (actor == NULL)
	{
		return; // Walls and floor
	}
	const Player *player = (Player *)JPH_CharacterVirtual_GetUserData(character);
	ioSettings->canPushCharacter = (actor->actorFlags & ACTOR_FLAG_CAN_PUSH_PLAYER) == ACTOR_FLAG_CAN_PUSH_PLAYER;
	if (player->hasHeldActor && player->heldActor->bodyId == bodyId)
	{
		ioSettings->canPushCharacter = false;
		ioSettings->canReceiveImpulses = false;
	}
	actor->definition->OnPlayerContactPersisted(actor, bodyId);
}

static void OnContactRemoved(const JPH_CharacterVirtual *character,
							 const JPH_BodyId bodyId,
							 JPH_SubShapeId /*subShapeId*/)
{
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(GetState()->level->physicsSystem);
	Actor *actor = (Actor *)JPH_BodyInterface_GetUserData(bodyInterface, bodyId);
	if (actor == NULL)
	{
		return; // Walls and floor
	}
	Player *player = (Player *)JPH_CharacterVirtual_GetUserData(character);
	if (player->hasHeldActor && player->heldActor->bodyId == bodyId)
	{
		player->canDropHeldActor = true;
	}
	actor->definition->OnPlayerContactRemoved(actor, bodyId);
}

static void OnContactSolve(const JPH_CharacterVirtual *character,
						   const JPH_BodyId bodyId,
						   const JPH_SubShapeId /*subShapeId*/,
						   const JPH_RVec3 * /*contactPosition*/,
						   const Vector3 * /*contactNormal*/,
						   const Vector3 * /*contactVelocity*/,
						   const JPH_PhysicsMaterial * /*contactMaterial*/,
						   const Vector3 *characterVelocity,
						   Vector3 *newCharacterVelocity)
{
	const Player *player = (Player *)JPH_CharacterVirtual_GetUserData(character);
	if (player->hasHeldActor && player->heldActor->bodyId == bodyId)
	{
		*newCharacterVelocity = *characterVelocity;
	}
}

static bool BodyFilterShouldCollide(const JPH_BodyId /*bodyId*/)
{
	const Player *player = (const Player *)JPH_CharacterVirtual_GetUserData(GetState()->level->player.joltCharacter);
	return !player->isNoclipActive;
}

static bool BodyFilterShouldCollideLocked(const JPH_Body * /*body*/)
{
	const Player *player = (const Player *)JPH_CharacterVirtual_GetUserData(GetState()->level->player.joltCharacter);
	return !player->isNoclipActive;
}

static const JPH_CharacterContactListener_Impl contactListenerImpl = {
	.OnContactAdded = OnContactAdded,
	.OnContactPersisted = OnContactPersisted,
	.OnContactRemoved = OnContactRemoved,
	.OnContactSolve = OnContactSolve,
};
static const JPH_BodyFilter_Impl bodyFilterImpl = {
	.ShouldCollide = BodyFilterShouldCollide,
	.ShouldCollideLocked = BodyFilterShouldCollideLocked,
};
static JPH_CharacterContactListener *contactListener;
static JPH_BodyFilter *bodyFilter;
static JPH_ShapeFilter *shapeFilter;


void PlayerPersistentStateInit()
{
	contactListener = JPH_CharacterContactListener_Create(&contactListenerImpl);
	bodyFilter = JPH_BodyFilter_Create(&bodyFilterImpl);
	shapeFilter = JPH_ShapeFilter_Create(NULL);
	actorRaycastBroadPhaseLayerFilter = JPH_BroadPhaseLayerFilter_Create(&actorRaycastBroadPhaseLayerFilterImpl);
	actorRaycastObjectLayerFilter = JPH_ObjectLayerFilter_Create(&actorRaycastObjectLayerFilterImpl);
}

void PlayerPersistentStateDestroy()
{
	JPH_CharacterContactListener_Destroy(contactListener);
	JPH_BodyFilter_Destroy(bodyFilter);
	JPH_ShapeFilter_Destroy(shapeFilter);
	JPH_BroadPhaseLayerFilter_Destroy(actorRaycastBroadPhaseLayerFilter);
	JPH_ObjectLayerFilter_Destroy(actorRaycastObjectLayerFilter);
}

void CreatePlayer(Level *level)
{
	JPH_Shape *shape = (JPH_Shape *)JPH_CapsuleShape_Create(0.25f, 0.25f);
	JPH_CharacterVirtualSettings characterSettings = {
		.base.maxSlopeAngle = degToRad(MAX_WALKABLE_SLOPE),
		.base.enhancedInternalEdgeRemoval = true,
		.base.shape = shape,
		.mass = 10.0f,
	};
	JPH_CharacterVirtualSettings_Init(&characterSettings);
	level->player.joltCharacter = JPH_CharacterVirtual_Create(&characterSettings,
															  &level->player.transform.position,
															  NULL,
															  0,
															  level->physicsSystem);
	JPH_CharacterVirtual_SetUserData(level->player.joltCharacter, (uint64_t)&level->player);
	JPH_CharacterVirtual_SetListener(level->player.joltCharacter, contactListener);
	level->player.hasHeldActor = false;
	level->player.canDropHeldActor = true;
	JPH_Shape_Destroy(shape);
}

void MovePlayer(const Player *player, float *distanceTraveled)
{
	Vector3 moveVec = Vector3_Zero;

	if (UseController())
	{
		moveVec.z = GetAxis(SDL_CONTROLLER_AXIS_LEFTY);
		moveVec.x = GetAxis(SDL_CONTROLLER_AXIS_LEFTX);
		if (fabsf(moveVec.x) < STICK_DEADZONE)
		{
			moveVec.x = 0;
		}
		if (fabsf(moveVec.z) < STICK_DEADZONE)
		{
			moveVec.z = 0;
		}
	} else
	{
		if (IsKeyPressed(SDL_SCANCODE_W))
		{
			moveVec.z -= 1;
		} else if (IsKeyPressed(SDL_SCANCODE_S))
		{
			moveVec.z += 1;
		}

		if (IsKeyPressed(SDL_SCANCODE_D))
		{
			moveVec.x += 1;
		} else if (IsKeyPressed(SDL_SCANCODE_A))
		{
			moveVec.x -= 1;
		}
	}

	if (moveVec.x != 0 || moveVec.z != 0)
	{
		Vector3_Normalized(&moveVec, &moveVec);
		*distanceTraveled = MOVE_SPEED;
		if (IsKeyPressed(SDL_SCANCODE_LCTRL) || GetAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 0.5)
		{
			*distanceTraveled = SLOW_MOVE_SPEED;
		} else if (player->isNoclipActive && (IsKeyPressed(SDL_SCANCODE_LSHIFT) || IsKeyPressed(SDL_SCANCODE_RSHIFT)))
		{
			*distanceTraveled = MOVE_SPEED * 2;
		} else
		{
			*distanceTraveled = MOVE_SPEED;
		}
		Vector3_MultiplyScalar(&moveVec, *distanceTraveled, &moveVec);

		if (player->isNoclipActive)
		{
			JPH_Quat_Rotate(&player->transform.rotation, &moveVec, &moveVec);
		} else
		{
			JPH_Quat playerRotation;
			JPH_Quat_Rotation(&Vector3_AxisY,
							  JPH_Quat_GetRotationAngle(&player->transform.rotation, &Vector3_AxisY),
							  &playerRotation);
			JPH_Quat_Rotate(&playerRotation, &moveVec, &moveVec);
		}
		JPH_CharacterVirtual_SetLinearVelocity(player->joltCharacter, &moveVec);
	} else
	{
		JPH_CharacterVirtual_SetLinearVelocity(player->joltCharacter, &Vector3_Zero);
	}
}

static inline Actor *GetTargetedActor(JPH_BodyInterface *bodyInterface, JPH_RayCastResult *raycastResult)
{
	const GlobalState *state = GetState();
	const JPH_NarrowPhaseQuery *narrowPhaseQuery = JPH_PhysicsSystem_GetNarrowPhaseQuery(state->level->physicsSystem);
	if (!JPH_NarrowPhaseQuery_CastRay_GAME(narrowPhaseQuery,
										   &state->level->player.transform,
										   actorRaycastMaxDistance,
										   raycastResult,
										   actorRaycastBroadPhaseLayerFilter,
										   actorRaycastObjectLayerFilter))
	{
		return NULL;
	}
	return (Actor *)JPH_BodyInterface_GetUserData(bodyInterface, raycastResult->bodyID);
}

void UpdatePlayer(Player *player, const JPH_PhysicsSystem *physicsSystem, const float deltaTime)
{
	if (player->hasHeldActor)
	{
		if ((IsKeyJustPressedPhys(SDL_SCANCODE_E) || IsButtonJustPressedPhys(SDL_CONTROLLER_BUTTON_A)) &&
			player->canDropHeldActor)
		{
			player->heldActor = NULL;
			player->hasHeldActor = false;
			crosshairColor = COLOR(0xFFFFCCCC);
		} else
		{
			Vector3 heldActorPosition;
			JPH_BodyInterface_GetPosition(player->heldActor->bodyInterface,
										  player->heldActor->bodyId,
										  &heldActorPosition);
			Vector3 heldActorPositionOffset;
			Vector3_Subtract(&heldActorPosition, &player->transform.position, &heldActorPositionOffset);
			if (Vector3_LengthSquared(&heldActorPositionOffset) > heldActorMaxDistanceSquared)
			{
				player->heldActor = NULL;
				player->hasHeldActor = false;
				crosshairColor = COLOR(0xFFFFCCCC);
			} else
			{
				Vector3 forward;
				JPH_Quat_Rotate(&player->transform.rotation, (Vector3[]){{0.0f, 0.0f, -offset}}, &forward);
				Vector3 offsetFromTarget;
				Vector3_Subtract(&forward, &heldActorPositionOffset, &offsetFromTarget);
				Vector3 heldActorLinearVelocity;
				Vector3_MultiplyScalar(&offsetFromTarget, smoothFactor, &heldActorLinearVelocity);

				JPH_Quat heldActorRotation;
				JPH_BodyInterface_GetRotation(player->heldActor->bodyInterface,
											  player->heldActor->bodyId,
											  &heldActorRotation);
				JPH_Quat targetRotation = {
					.y = player->transform.rotation.y,
					.w = player->transform.rotation.w,
				};
				JPH_Quat_Lerp(&heldActorRotation, &targetRotation, 0.2f, &targetRotation);
				JPH_Quat_Normalized(&targetRotation, &targetRotation);

				JPH_BodyInterface_SetLinearAndAngularVelocity(player->heldActor->bodyInterface,
															  player->heldActor->bodyId,
															  &heldActorLinearVelocity,
															  &Vector3_Zero);
				JPH_BodyInterface_SetRotation(player->heldActor->bodyInterface,
											  player->heldActor->bodyId,
											  &targetRotation,
											  JPH_Activation_DontActivate);
			}
		}
	} else
	{
		JPH_RayCastResult raycastResult = {};
		player->targetedActor = GetTargetedActor(JPH_PhysicsSystem_GetBodyInterface(physicsSystem), &raycastResult);
		if (player->targetedActor)
		{
			if ((player->targetedActor->actorFlags & ACTOR_FLAG_ENEMY) == ACTOR_FLAG_ENEMY)
			{
				crosshairColor = COLOR(0xFFFF0000);
				if (IsMouseButtonJustPressedPhys(SDL_BUTTON_LEFT) || IsButtonJustPressedPhys(SDL_CONTROLLER_BUTTON_X))
				{
					RemoveActor(player->targetedActor);
					player->targetedActor = NULL;
					crosshairColor = COLOR(0xFFFFCCCC);
				}
			} else if (((player->targetedActor->actorFlags & ACTOR_FLAG_CAN_BE_HELD) == ACTOR_FLAG_CAN_BE_HELD) &&
					   (raycastResult.fraction * actorRaycastMaxDistance < 1.0f))
			{
				crosshairColor = COLOR(0xFF006600);
				if (IsKeyJustPressedPhys(SDL_SCANCODE_E) || IsButtonJustPressedPhys(SDL_CONTROLLER_BUTTON_A))
				{
					player->heldActor = player->targetedActor;
					player->hasHeldActor = true;
					crosshairColor = COLOR(0x00FF0000);
				}
			} else
			{
				crosshairColor = COLOR(0xFFFFCCCC);
			}
		} else
		{
			crosshairColor = COLOR(0xFFFFCCCC);
		}
	}
	if (IsKeyJustReleasedPhys(SDL_SCANCODE_V))
	{
		player->isNoclipActive = !player->isNoclipActive;
	}
	const JPH_ExtendedUpdateSettings extendedUpdateSettings = {
		.stickToFloorStepDown.y = player->isNoclipActive ? 0.0f : -0.1f,
		.walkStairsStepUp.y = player->isNoclipActive ? 0.0f : 0.1f,
		.walkStairsMinStepForward = 0.02f,
		.walkStairsStepForwardTest = 0.15f,
		.walkStairsCosAngleForwardContact = cosf(degToRad(75)),
		.walkStairsStepDownExtra = Vector3_Zero,
	};
	JPH_CharacterVirtual_ExtendedUpdate(player->joltCharacter,
										deltaTime,
										&extendedUpdateSettings,
										OBJECT_LAYER_PLAYER,
										physicsSystem,
										bodyFilter,
										shapeFilter);
}

const Color *GetCrosshairColor()
{
	return &crosshairColor;
}

void DPrintPlayer(const Level *level)
{
	DPrintF("Position: (%.2f, %.2f, %.2f)",
			COLOR_WHITE,
			false,
			level->player.transform.position.x,
			level->player.transform.position.y,
			level->player.transform.position.z);
	Vector3 playerVelocity;
	JPH_CharacterVirtual_GetLinearVelocity(level->player.joltCharacter, &playerVelocity);
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
	JPH_Quat_GetEulerAngles(&level->player.transform.rotation, &eulerAngles);
	JPH_Quat_GetAxisAngle(&level->player.transform.rotation, &axis, &angle);
	DPrintF("Rotation: (%.4f, %.4f) (%.2fdeg, %.2fdeg)\nRotation Axis: (%.4f, %.4f, %.4f)\nRotation Angle: %.4f "
			"(%.2fdeg)",
			COLOR_WHITE,
			false,
			level->player.transform.rotation.x,
			fabsf(level->player.transform.rotation.y),
			radToDeg(level->player.transform.rotation.x),
			radToDeg(fabsf(level->player.transform.rotation.y)),
			axis.x,
			axis.y,
			axis.z,
			angle,
			radToDeg(angle));
	DPrintF("%s Actor: %p",
			COLOR_WHITE,
			false,
			level->player.hasHeldActor ? "Held" : "Targeted",
			level->player.targetedActor);
}
