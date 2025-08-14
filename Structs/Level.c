//
// Created by droc101 on 4/21/2024.
//

#include "Level.h"
#include <joltc.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/List.h"
#include "../Helpers/Core/Physics/Physics.h"
#include "../Helpers/Core/Physics/Player.h"
#include "Actor.h"
#include "GlobalState.h"
#include "Wall.h"

Level *CreateLevel(void)
{
	Level *level = calloc(1, sizeof(Level));
	CheckAlloc(level);
	ListInit(level->actors, LIST_POINTER);
	ListInit(level->walls, LIST_POINTER);
	PhysicsInitLevel(level);
	CreatePlayer(level);
	strncpy(level->ceilOrSkyTex, "texture/level/sky_test.gtex", 28);
	strncpy(level->floorTex, "texture/level/floor_test.gtex", 30);
	strncpy(level->music, "none", 5);
	level->fogColor = 0xff000000;
	level->fogStart = 10;
	level->fogEnd = 30;
	ListInit(level->namedActorNames, LIST_POINTER);
	ListInit(level->namedActorPointers, LIST_POINTER);
	return level;
}

void DestroyLevel(Level *level)
{
	for (size_t i = 0; i < level->actors.length; i++)
	{
		Actor *actor = ListGetPointer(level->actors, i);
		FreeActor(actor);
	}
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(level->physicsSystem);
	for (size_t i = 0; i < level->walls.length; i++)
	{
		Wall *wall = ListGetPointer(level->walls, i);
		FreeWall(bodyInterface, wall);
	}

	PhysicsDestroyLevel(level, bodyInterface);

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

void NameActor(Actor *actor, const char *name, Level *l)
{
	char *nameCopy = strdup(name);
	ListAdd(l->namedActorNames, nameCopy);
	ListAdd(l->namedActorPointers, actor);
}

Actor *GetActorByName(const char *name, const Level *l)
{
	ListLock(l->namedActorNames);
	for (size_t i = 0; i < l->namedActorNames.length; i++)
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
	for (size_t i = 0; i < l->namedActorNames.length; i++)
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
