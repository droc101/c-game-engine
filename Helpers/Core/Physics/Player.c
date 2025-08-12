//
// Created by NBT22 on 7/31/25.
//

#include "Player.h"
#include "../../../Debug/DPrint.h"
#include "../../../Structs/Actor.h"
#include "../../../Structs/GlobalState.h"
#include "../../../Structs/Vector2.h"
#include "../Input.h"
#include "../MathEx.h"

static void OnContactAdded(const JPH_CharacterVirtual * /*character*/,
						   const JPH_BodyId bodyId,
						   JPH_SubShapeId /*subShapeId*/,
						   const JPH_RVec3 * /*contactPosition*/,
						   const Vector3 * /*contactNormal*/,
						   JPH_CharacterContactSettings * /*ioSettings*/)
{
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(GetState()->level->physicsSystem);
	Actor *actor = (Actor *)JPH_BodyInterface_GetUserData(bodyInterface, bodyId);
	if (actor == NULL)
	{
		return; // Walls and floor
	}
	if (actor->OnPlayerContactAdded)
	{
		actor->OnPlayerContactAdded(actor, bodyId);
	}
}
static void OnContactPersisted(const JPH_CharacterVirtual * /*character*/,
							   const JPH_BodyId bodyId,
							   JPH_SubShapeId /*subShapeId*/,
							   const JPH_RVec3 * /*contactPosition*/,
							   const Vector3 * /*contactNormal*/,
							   JPH_CharacterContactSettings * /*ioSettings*/)
{
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(GetState()->level->physicsSystem);
	Actor *actor = (Actor *)JPH_BodyInterface_GetUserData(bodyInterface, bodyId);
	if (actor == NULL)
	{
		return; // Walls and floor
	}
	if (actor->OnPlayerContactPersisted)
	{
		actor->OnPlayerContactPersisted(actor, bodyId);
	}
}
static void OnContactRemoved(const JPH_CharacterVirtual * /*character*/,
							 const JPH_BodyId bodyId,
							 JPH_SubShapeId /*subShapeId*/)
{
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(GetState()->level->physicsSystem);
	Actor *actor = (Actor *)JPH_BodyInterface_GetUserData(bodyInterface, bodyId);
	if (actor == NULL)
	{
		return; // Walls and floor
	}
	if (actor->OnPlayerContactRemoved)
	{
		actor->OnPlayerContactRemoved(actor, bodyId);
	}
}


static JPH_CharacterContactListener *contactListener;
static JPH_CharacterContactListener_Impl contactListenerImpl = {
	.OnContactAdded = OnContactAdded,
	.OnContactPersisted = OnContactPersisted,
	.OnContactRemoved = OnContactRemoved,
};
static JPH_BodyFilter *bodyFilter;
static JPH_ShapeFilter *shapeFilter;

void PlayerPersistentStateInit()
{
	contactListener = JPH_CharacterContactListener_Create(&contactListenerImpl);
	bodyFilter = JPH_BodyFilter_Create(NULL);
	shapeFilter = JPH_ShapeFilter_Create(NULL);
}

void PlayerPersistentStateDestroy()
{
	JPH_CharacterContactListener_Destroy(contactListener);
	JPH_BodyFilter_Destroy(bodyFilter);
	JPH_ShapeFilter_Destroy(shapeFilter);
}

void CreatePlayerCollider(Level *level)
{
	JPH_CharacterVirtualSettings characterSettings = {
		.base.maxSlopeAngle = degToRad(MAX_WALKABLE_SLOPE),
		.base.enhancedInternalEdgeRemoval = true,
		.base.shape = (const JPH_Shape *)JPH_CapsuleShape_Create(0.25f, 0.25f),
		.mass = 10.0f,
	};
	JPH_CharacterVirtualSettings_Init(&characterSettings);
	level->player.joltCharacter = JPH_CharacterVirtual_Create(&characterSettings,
															  &level->player.transform.position,
															  NULL,
															  0,
															  level->physicsSystem);
	JPH_CharacterVirtual_SetListener(level->player.joltCharacter, contactListener);
}

void MovePlayer(const Player *player, float *distanceTraveled)
{
	Vector2 moveVec = v2s(0);

	if (UseController())
	{
		moveVec.y = GetAxis(SDL_CONTROLLER_AXIS_LEFTX);
		moveVec.x = -GetAxis(SDL_CONTROLLER_AXIS_LEFTY);
		if (fabsf(moveVec.x) < STICK_DEADZONE)
		{
			moveVec.x = 0;
		}
		if (fabsf(moveVec.y) < STICK_DEADZONE)
		{
			moveVec.y = 0;
		}
	} else
	{
		if (IsKeyPressed(SDL_SCANCODE_W))
		{
			moveVec.y -= 1;
		} else if (IsKeyPressed(SDL_SCANCODE_S))
		{
			moveVec.y += 1;
		}

		if (IsKeyPressed(SDL_SCANCODE_D))
		{
			moveVec.x += 1;
		} else if (IsKeyPressed(SDL_SCANCODE_A))
		{
			moveVec.x -= 1;
		}
	}

	if (moveVec.x != 0 || moveVec.y != 0)
	{
		moveVec = Vector2Normalize(moveVec);
		*distanceTraveled = MOVE_SPEED;
		if (IsKeyPressed(SDL_SCANCODE_LCTRL) || GetAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 0.5)
		{
			*distanceTraveled = SLOW_MOVE_SPEED;
		}
		moveVec = Vector2Rotate(Vector2Scale(moveVec, *distanceTraveled), -player->transform.rotation.y);
	}
	JPH_CharacterVirtual_SetLinearVelocity(player->joltCharacter, (Vector3[]){{moveVec.x, 0.0f, moveVec.y}});
}

void Update(const Player *player, const JPH_PhysicsSystem *physicsSystem, const float deltaTime)
{
	JPH_CharacterVirtual_Update(player->joltCharacter,
								deltaTime,
								OBJECT_LAYER_PLAYER,
								physicsSystem,
								bodyFilter,
								shapeFilter);
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
	DPrintF("Rotation: (%.4f, %.4f) (%.2fdeg, %.2fdeg)",
			COLOR_WHITE,
			false,
			level->player.transform.rotation.x,
			fabsf(level->player.transform.rotation.y),
			radToDeg(level->player.transform.rotation.x),
			radToDeg(fabsf(level->player.transform.rotation.y)));
}
