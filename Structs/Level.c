//
// Created by droc101 on 4/21/2024.
//

#include "Level.h"
#include <string.h>
#include "../Debug/JoltDebugRenderer.h"
#include "../defines.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Player.h"
#include "Actor.h"
#include "GlobalState.h"
#include "Wall.h"

static void InitJolt(Level *level)
{
	JPH_BroadPhaseLayerInterface *layerInterface = JPH_BroadPhaseLayerInterfaceTable_Create(OBJECT_LAYER_MAX,
																							BROADPHASE_LAYER_MAX);
	JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(layerInterface,
																 OBJECT_LAYER_STATIC,
																 BROADPHASE_LAYER_STATIC);
	JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(layerInterface,
																 OBJECT_LAYER_SENSOR,
																 BROADPHASE_LAYER_STATIC);
	JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(layerInterface,
																 OBJECT_LAYER_DYNAMIC,
																 BROADPHASE_LAYER_DYNAMIC);
	JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(layerInterface,
																 OBJECT_LAYER_PLAYER,
																 BROADPHASE_LAYER_DYNAMIC);

	JPH_ObjectLayerPairFilter *layerPairFilter = JPH_ObjectLayerPairFilterTable_Create(OBJECT_LAYER_MAX);
	JPH_ObjectLayerPairFilterTable_EnableCollision(layerPairFilter, OBJECT_LAYER_DYNAMIC, OBJECT_LAYER_STATIC);
	JPH_ObjectLayerPairFilterTable_EnableCollision(layerPairFilter, OBJECT_LAYER_DYNAMIC, OBJECT_LAYER_DYNAMIC);
	JPH_ObjectLayerPairFilterTable_EnableCollision(layerPairFilter, OBJECT_LAYER_DYNAMIC, OBJECT_LAYER_SENSOR);
	JPH_ObjectLayerPairFilterTable_EnableCollision(layerPairFilter, OBJECT_LAYER_PLAYER, OBJECT_LAYER_STATIC);
	JPH_ObjectLayerPairFilterTable_EnableCollision(layerPairFilter, OBJECT_LAYER_PLAYER, OBJECT_LAYER_DYNAMIC);
	JPH_ObjectLayerPairFilterTable_EnableCollision(layerPairFilter, OBJECT_LAYER_PLAYER, OBJECT_LAYER_SENSOR);

	const JPH_PhysicsSystemSettings physicsSystemSettings = {
		.broadPhaseLayerInterface = layerInterface,
		.objectLayerPairFilter = layerPairFilter,
		.objectVsBroadPhaseLayerFilter = JPH_ObjectVsBroadPhaseLayerFilterTable_Create(layerInterface,
																					   BROADPHASE_LAYER_MAX,
																					   layerPairFilter,
																					   OBJECT_LAYER_MAX),
	};
	level->physicsSystem = JPH_PhysicsSystem_Create(&physicsSystemSettings);

	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(level->physicsSystem);
	const JPH_Plane plane = {
		.normal.y = 1,
	};
	JPH_BodyCreationSettings *floorSettings = JPH_BodyCreationSettings_Create3(
			(const JPH_Shape *)JPH_PlaneShape_Create(&plane, NULL, 100),
			(Vector3[]){{0.0f, -0.5f, 0.0f}},
			&JPH_Quat_Identity,
			JPH_MotionType_Static,
			OBJECT_LAYER_STATIC);

	level->floorBodyId = JPH_BodyInterface_CreateAndAddBody(bodyInterface, floorSettings, JPH_Activation_DontActivate);
	JPH_BodyCreationSettings_Destroy(floorSettings);
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
