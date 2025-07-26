//
// Created by droc101 on 4/21/2024.
//

#include "Level.h"
#include <string.h>
#include "../Debug/JoltDebugRenderer.h"
#include "../defines.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/MathEx.h"
#include "Actor.h"
#include "GlobalState.h"
#include "Wall.h"

static void InitJolt(Level *level)
{
	JPH_BroadPhaseLayerInterface *broadPhaseLayerInterface = JPH_BroadPhaseLayerInterfaceTable_Create(3, 2);
	JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(broadPhaseLayerInterface,
																 OBJECT_LAYER_STATIC,
																 BROADPHASE_LAYER_STATIC);
	JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(broadPhaseLayerInterface,
																 OBJECT_LAYER_DYNAMIC,
																 BROADPHASE_LAYER_DYNAMIC);
	JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(broadPhaseLayerInterface,
																 OBJECT_LAYER_PLAYER,
																 BROADPHASE_LAYER_DYNAMIC);

	JPH_ObjectLayerPairFilter *objectLayerPairFilter = JPH_ObjectLayerPairFilterTable_Create(3);
	JPH_ObjectLayerPairFilterTable_EnableCollision(objectLayerPairFilter, OBJECT_LAYER_DYNAMIC, OBJECT_LAYER_STATIC);
	JPH_ObjectLayerPairFilterTable_EnableCollision(objectLayerPairFilter, OBJECT_LAYER_DYNAMIC, OBJECT_LAYER_DYNAMIC);
	JPH_ObjectLayerPairFilterTable_EnableCollision(objectLayerPairFilter, OBJECT_LAYER_PLAYER, OBJECT_LAYER_STATIC);
	JPH_ObjectLayerPairFilterTable_EnableCollision(objectLayerPairFilter, OBJECT_LAYER_PLAYER, OBJECT_LAYER_DYNAMIC);

	const JPH_PhysicsSystemSettings physicsSystemSettings = {
		.broadPhaseLayerInterface = broadPhaseLayerInterface,
		.objectLayerPairFilter = objectLayerPairFilter,
		.objectVsBroadPhaseLayerFilter = JPH_ObjectVsBroadPhaseLayerFilterTable_Create(broadPhaseLayerInterface,
																					   2,
																					   objectLayerPairFilter,
																					   3),
	};
	level->physicsSystem = JPH_PhysicsSystem_Create(&physicsSystemSettings);

	const JPH_Plane plane = {
		.normal.y = 1,
	};
	JPH_BodyCreationSettings *floorSettings = JPH_BodyCreationSettings_Create3(
			(const JPH_Shape *)JPH_PlaneShape_Create(&plane, NULL, 100),
			(Vector3[]){{0.0f, -0.5f, 0.0f}},
			NULL, // Because joltc doesn't expose JPH::Quat::sIdentity() this is what we have to do to get the identity quaternion (which is [0, 0, 0, 1])
			JPH_MotionType_Static,
			OBJECT_LAYER_STATIC);

	level->floorBodyId = JPH_BodyInterface_CreateAndAddBody(JPH_PhysicsSystem_GetBodyInterface(level->physicsSystem),
															floorSettings,
															JPH_Activation_DontActivate);
	JPH_BodyCreationSettings_Destroy(floorSettings);
}

void CreatePlayerCollider(Level *level)
{
	const JPH_CharacterSettings characterSettings = {
		.base.up = {0.0f, 1.0f, 0.0f},
		.base.supportingVolume.normal.y = 1.0f,
		.base.supportingVolume.distance = -1.0e10f, // Default value in Jolt
		.base.maxSlopeAngle = degToRad(MAX_WALKABLE_SLOPE),
		.base.enhancedInternalEdgeRemoval = true,
		.base.shape = (const JPH_Shape *)JPH_CapsuleShape_Create(0.25f, 0.25f),
		.layer = OBJECT_LAYER_PLAYER,
		.mass = 80.0f,
		.friction = 25.0f,
		.gravityFactor = 1.0f,
		.allowedDOFs = JPH_AllowedDOFs_TranslationX | JPH_AllowedDOFs_TranslationY | JPH_AllowedDOFs_TranslationZ,
	};
	level->player.joltCharacter = JPH_Character_Create(&characterSettings,
													   &JPH_Vec3_Zero,
													   NULL,
													   0,
													   level->physicsSystem);
	JPH_Character_AddToPhysicsSystem(level->player.joltCharacter, JPH_Activation_DontActivate, false);
}

Level *CreateLevel(void)
{
	Level *level = calloc(1, sizeof(Level));
	CheckAlloc(level);
	ListInit(level->actors, LIST_POINTER);
	ListInit(level->walls, LIST_POINTER);
	InitJolt(level);
	CreatePlayerCollider(level);
	strncpy(level->ceilOrSkyTex, "texture/level_sky_test.gtex", 28);
	strncpy(level->floorTex, "texture/level_floor_test.gtex", 30);
	strncpy(level->music, "none", 5);
	level->fogColor = 0xff000000;
	level->fogStart = 10;
	level->fogEnd = 30;
	strncpy(level->name, "Unnamed Level", 32);
	level->courseNum = -1;
	ListInit(level->namedActorNames, LIST_POINTER);
	ListInit(level->namedActorPointers, LIST_POINTER);
	return level;
}

void DestroyLevel(Level *level)
{
	for (int i = 0; i < level->actors.length; i++)
	{
		Actor *actor = ListGetPointer(level->actors, i);
		FreeActor(actor);
	}
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(level->physicsSystem);
	for (int i = 0; i < level->walls.length; i++)
	{
		Wall *wall = ListGetPointer(level->walls, i);
		FreeWall(bodyInterface, wall);
	}

	JPH_BodyInterface_RemoveAndDestroyBody(bodyInterface, level->floorBodyId);
	JPH_Character_RemoveFromPhysicsSystem(level->player.joltCharacter, true);

	JPH_PhysicsSystem_Destroy(level->physicsSystem);

	ListAndContentsFree(level->namedActorNames);
	ListFree(level->namedActorPointers);
	ListFree(level->actors);
	ListFree(level->walls);
	free(level);
	level = NULL;
}

void AddActor(Actor *actor)
{
	Level *l = GetState()->level;
	ListAdd(l->actors, actor);
}

void RemoveActor(Actor *actor)
{
	Level *l = GetState()->level;
	ActorFireOutput(actor, ACTOR_KILLED_OUTPUT, PARAM_NONE);

	// Remove the actor from the named actor lists if it's there
	const size_t nameIdx = ListFind(l->namedActorPointers, actor);
	if (nameIdx != -1)
	{
		char *name = ListGetPointer(l->namedActorNames, nameIdx);
		free(name);
		ListRemoveAt(l->namedActorNames, nameIdx);
		ListRemoveAt(l->namedActorPointers, nameIdx);
	}

	const size_t idx = ListFind(l->actors, actor);
	if (idx == -1)
	{
		return;
	}
	ListRemoveAt(l->actors, idx);
	FreeActor(actor);
}

void NameActor(Actor *actor, const char *name, Level *l)
{
	char *nameCopy = strdup(name);
	ListAdd(l->namedActorNames, nameCopy);
	ListAdd(l->namedActorPointers, actor);
}

Actor *GetActorByName(const char *name, const Level *l)
{
	ListLock(l->namedActorNames);
	for (int i = 0; i < l->namedActorNames.length; i++)
	{
		const char *actorName = ListGetPointer(l->namedActorNames, i);
		if (strcmp(actorName, name) == 0)
		{
			Actor *a = ListGetPointer(l->namedActorPointers, i);
			ListUnlock(l->namedActorNames);
			return a;
		}
	}
	ListUnlock(l->namedActorNames);
	return NULL;
}

void GetActorsByName(const char *name, const Level *l, List *actors)
{
	ListInit(*actors, LIST_POINTER);
	ListLock(l->namedActorNames);
	for (int i = 0; i < l->namedActorNames.length; i++)
	{
		const char *actorName = ListGetPointer(l->namedActorNames, i);
		if (strcmp(actorName, name) == 0)
		{
			Actor *a = ListGetPointer(l->namedActorPointers, i);
			ListAdd(*actors, a);
		}
	}
	ListUnlock(l->namedActorNames);
}
