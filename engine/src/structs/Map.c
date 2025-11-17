//
// Created by droc101 on 4/21/2024.
//

#include <engine/assets/AssetReader.h>
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
	strncpy(map->skyTexture, TEXTURE("level/sky_test"), 28);
	map->fogColor = 0xff000000;
	map->fogStart = 2000;
	map->fogEnd = 2500;
	ListInit(map->namedActorNames, LIST_POINTER);
	ListInit(map->namedActorPointers, LIST_POINTER);
	return map;
}

void DestroyMap(Map *map)
{
	for (size_t i = 0; i < map->actors.length; i++)
	{
		Actor *actor = ListGetPointer(map->actors, i);
		FreeActor(actor);
	}
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(map->physicsSystem);

	PhysicsDestroyMap(map, bodyInterface);

	ListAndContentsFree(map->namedActorNames);
	ListFree(map->namedActorPointers);
	ListFree(map->actors);
	free(map);
	map = NULL;
}

void AddActor(Actor *actor)
{
	Map *l = GetState()->map;
	ListAdd(l->actors, actor);
}

void RemoveActor(Actor *actor)
{
	Map *l = GetState()->map;
	ActorFireOutput(actor, ACTOR_OUTPUT_KILLED, PARAM_NONE);

	// Remove the actor from the named actor lists if it's there
	const size_t nameIdx = ListFind(l->namedActorPointers, actor);
	if (nameIdx != SIZE_MAX)
	{
		char *name = ListGetPointer(l->namedActorNames, nameIdx);
		free(name);
		ListRemoveAt(l->namedActorNames, nameIdx);
		ListRemoveAt(l->namedActorPointers, nameIdx);
	}

	const size_t idx = ListFind(l->actors, actor);
	if (idx == SIZE_MAX)
	{
		return;
	}
	ListRemoveAt(l->actors, idx);
	FreeActor(actor);
}

void NameActor(Actor *actor, const char *name, Map *l)
{
	char *nameCopy = strdup(name);
	ListAdd(l->namedActorNames, nameCopy);
	ListAdd(l->namedActorPointers, actor);
}

Actor *GetActorByName(const char *name, const Map *mapo)
{
	ListLock(mapo->namedActorNames);
	for (size_t i = 0; i < mapo->namedActorNames.length; i++)
	{
		const char *actorName = ListGetPointer(mapo->namedActorNames, i);
		if (strcmp(actorName, name) == 0)
		{
			Actor *a = ListGetPointer(mapo->namedActorPointers, i);
			ListUnlock(mapo->namedActorNames);
			return a;
		}
	}
	ListUnlock(mapo->namedActorNames);
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
			Actor *a = ListGetPointer(map->namedActorPointers, i);
			ListAdd(*actors, a);
		}
	}
	ListUnlock(map->namedActorNames);
}

void RenderMap(const Map *map, const Camera *camera)
{
	JoltDebugRendererDrawBodies(map->physicsSystem);
	RenderMap3D(map, camera);

	for (size_t i = 0; i < map->actors.length; i++)
	{
		Actor *a = ListGetPointer(map->actors, i);
		a->definition->RenderUi(a);
	}
}
