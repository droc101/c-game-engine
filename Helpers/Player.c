//
// Created by NBT22 on 7/31/25.
//

#include "Player.h"

#include "../Structs/Actor.h"
#include "../Structs/GlobalState.h"
#include "Core/MathEx.h"

static void OnContactAdded(const JPH_CharacterVirtual * /*character*/,
						   const JPH_BodyId bodyId,
						   JPH_SubShapeId /*subShapeId*/,
						   const JPH_RVec3 * /*contactPosition*/,
						   const JPH_Vec3 * /*contactNormal*/,
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
		actor->OnPlayerContactAdded(actor);
	}
}
static void OnContactPersisted(const JPH_CharacterVirtual * /*character*/,
							   const JPH_BodyId bodyId,
							   JPH_SubShapeId /*subShapeId*/,
							   const JPH_RVec3 * /*contactPosition*/,
							   const JPH_Vec3 * /*contactNormal*/,
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
		actor->OnPlayerContactPersisted(actor);
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
		actor->OnPlayerContactRemoved(actor);
	}
}


static JPH_CharacterContactListener *contactListener;
static JPH_CharacterContactListener_Procs contactListenerProcs = {
	.OnContactAdded = OnContactAdded,
	.OnContactPersisted = OnContactPersisted,
	.OnContactRemoved = OnContactRemoved,
};

void PlayerContactListenerInit()
{
	contactListener = JPH_CharacterContactListener_Create();
	JPH_CharacterContactListener_SetProcs(contactListener, &contactListenerProcs);
}

void PlayerContactListenerDestroy()
{
	JPH_CharacterContactListener_Destroy(contactListener);
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
