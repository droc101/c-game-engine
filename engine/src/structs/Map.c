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

Map *CreateLevel(void)
{
	Map *level = calloc(1, sizeof(Map));
	CheckAlloc(level);
	ListInit(level->actors, LIST_POINTER);
	PhysicsInitLevel(level);
	CreatePlayer(&level->player, level->physicsSystem);
	strncpy(level->skyTexture, TEXTURE("level/sky_test"), 28);
	level->fogColor = 0xff000000;
	level->fogStart = 10;
	level->fogEnd = 30;
	ListInit(level->namedActorNames, LIST_POINTER);
	ListInit(level->namedActorPointers, LIST_POINTER);
	return level;
}

void DestroyLevel(Map *level)
{
	for (size_t i = 0; i < level->actors.length; i++)
	{
		Actor *actor = ListGetPointer(level->actors, i);
		FreeActor(actor);
	}
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(level->physicsSystem);

	PhysicsDestroyLevel(level, bodyInterface);

	ListAndContentsFree(level->namedActorNames);
	ListFree(level->namedActorPointers);
	ListFree(level->actors);
	free(level);
	level = NULL;
}

void AddActor(Actor *actor)
{
	Map *l = GetState()->level;
	ListAdd(l->actors, actor);
}

void RemoveActor(Actor *actor)
{
	Map *l = GetState()->level;
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

Actor *GetActorByName(const char *name, const Map *l)
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

void GetActorsByName(const char *name, const Map *l, List *actors)
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

void RenderLevel(const Map *level, const Camera *camera)
{
	JoltDebugRendererDrawBodies(level->physicsSystem);
	RenderLevel3D(level, camera);

	for (size_t i = 0; i < level->actors.length; i++)
	{
		Actor *a = ListGetPointer(level->actors, i);
		a->definition->RenderUi(a);
	}
}
