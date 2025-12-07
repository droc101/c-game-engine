//
// Created by droc101 on 4/21/2024.
//

#include <engine/debug/JoltDebugRenderer.h>
#include <engine/graphics/Drawing.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/Camera.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Param.h>
#include <engine/structs/Player.h>
#include <engine/structs/Wall.h>
#include <engine/subsystem/Error.h>
#include <joltc/joltc.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Map *CreateMap(void)
{
	Map *map = calloc(1, sizeof(Map));
	CheckAlloc(map);
	ListInit(map->actors, LIST_POINTER);
	PhysicsInitMap(map);
	CreatePlayer(&map->player, map->physicsSystem);
	map->fogColor = 0xff000000;
	map->fogStart = 2000;
	map->fogEnd = 2500;
	map->discordRpcIcon = NULL;
	map->discordRpcName = NULL;
	map->skyTexture = NULL;
	ListInit(map->namedActorNames, LIST_POINTER);
	ListInit(map->namedActorPointers, LIST_POINTER);
	ListInit(map->joltBodies, LIST_UINT32);
	return map;
}

void DestroyMap(Map *map)
{
	for (size_t i = 0; i < map->actors.length; i++)
	{
		FreeActor(ListGetPointer(map->actors, i));
	}

	for (size_t i = 0; i < map->modelCount; i++)
	{
		const MapModel *model = map->models + i;
		free(model->vertices);
		free(model->indices);
	}
	free(map->models);
	map->models = NULL;

	free(map->skyTexture);
	free(map->discordRpcIcon);
	free(map->discordRpcName);

	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(map->physicsSystem);

	for (size_t i = 0; i < map->joltBodies.length; i++)
	{
		JPH_BodyInterface_RemoveAndDestroyBody(bodyInterface, ListGetUint32(map->joltBodies, i));
	}
	ListFree(map->joltBodies);

	PhysicsDestroyMap(map, bodyInterface);

	ListAndContentsFree(map->namedActorNames);
	ListFree(map->namedActorPointers);
	ListFree(map->actors);
	free(map);
}

void AddActor(Actor *actor)
{
	ListAdd(GetState()->map->actors, actor);
}

void RemoveActor(Actor *actor)
{
	Map *map = GetState()->map;
	ActorFireOutput(actor, ACTOR_OUTPUT_KILLED, PARAM_NONE);

	// Remove the actor from the named actor lists if it's there
	const size_t nameIdx = ListFind(map->namedActorPointers, actor);
	if (nameIdx != SIZE_MAX)
	{
		free(ListGetPointer(map->namedActorNames, nameIdx));
		ListRemoveAt(map->namedActorNames, nameIdx);
		ListRemoveAt(map->namedActorPointers, nameIdx);
	}

	const size_t idx = ListFind(map->actors, actor);
	if (idx == SIZE_MAX)
	{
		return;
	}
	ListRemoveAt(map->actors, idx);
	FreeActor(actor);
}

void NameActor(Actor *actor, const char *name, Map *map)
{
	ListAdd(map->namedActorNames, strdup(name));
	ListAdd(map->namedActorPointers, actor);
}

Actor *GetActorByName(const char *name, const Map *map)
{
	ListLock(map->namedActorNames);
	for (size_t i = 0; i < map->namedActorNames.length; i++)
	{
		if (strcmp(ListGetPointer(map->namedActorNames, i), name) == 0)
		{
			Actor *actor = ListGetPointer(map->namedActorPointers, i);
			ListUnlock(map->namedActorNames);
			return actor;
		}
	}
	ListUnlock(map->namedActorNames);
	return NULL;
}

void GetActorsByName(const char *name, const Map *map, List *actors)
{
	ListInit(*actors, LIST_POINTER);
	ListLock(map->namedActorNames);
	for (size_t i = 0; i < map->namedActorNames.length; i++)
	{
		const char *actorName = ListGetPointer(map->namedActorNames, i);
		if (strcmp(actorName, name) == 0)
		{
			ListAdd(*actors, ListGetPointer(map->namedActorPointers, i));
		}
	}
	ListUnlock(map->namedActorNames);
}

void RenderMap(const Map *map, const Camera *camera)
{
	JoltDebugRendererDrawBodies(map->physicsSystem);
	RenderMap3D(map, camera);

	ListLock(map->actors);
	for (size_t i = 0; i < map->actors.length; i++)
	{
		Actor *actor = ListGetPointer(map->actors, i);
		actor->definition->RenderUi(actor);
	}
	ListUnlock(map->actors);
}
