//
// Created by droc101 on 4/21/2024.
//

#include "Level.h"
#include <box2d/box2d.h>
#include <string.h>
#include "../defines.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Graphics/Drawing.h"
#include "Actor.h"
#include "GlobalState.h"
#include "Vector2.h"

void CreatePlayerCollider(Level *level)
{
	b2BodyDef playerBodyDef = b2DefaultBodyDef();
	playerBodyDef.type = b2_dynamicBody;
	playerBodyDef.position = level->player.pos;
	playerBodyDef.fixedRotation = true;
	playerBodyDef.linearDamping = 12;
	level->player.bodyId = b2CreateBody(level->worldId, &playerBodyDef);
	const b2Circle playerShape = {
		.center = level->player.pos,
		.radius = 0.25f,
	};
	b2ShapeDef playerShapeDef = b2DefaultShapeDef();
	playerShapeDef.filter.categoryBits = COLLISION_GROUP_PLAYER;
	b2CreateCircleShape(level->player.bodyId, &playerShapeDef, &playerShape);
}

Level *CreateLevel()
{
	Level *l = malloc(sizeof(Level));
	CheckAlloc(l);
	ListCreate(&l->actors);
	ListCreate(&l->walls);
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity.y = 0;
	l->worldId = b2CreateWorld(&worldDef);
	l->player.pos = v2s(0);
	l->player.angle = 0;
	CreatePlayerCollider(l);
	l->hasCeiling = false;
	strncpy(l->ceilOrSkyTex, "texture/level_sky_test.gtex", 28);
	strncpy(l->floorTex, "texture/level_floor_test.gtex", 30);
	strncpy(l->music, "none", 5);
	l->fogColor = 0xff000000;
	l->fogStart = 10;
	l->fogEnd = 30;
	strncpy(l->name, "Unnamed Level", 32);
	l->courseNum = -1;
	l->ioProxy = NULL;
	ListCreate(&l->namedActorNames);
	ListCreate(&l->namedActorPointers);
	return l;
}

void DestroyLevel(Level *l)
{
	for (int i = 0; i < l->actors.length; i++)
	{
		Actor *a = ListGet(l->actors, i);
		FreeActor(a);
	}

	b2DestroyWorld(l->worldId);

	ListAndContentsFree(&l->walls, false);
	ListAndContentsFree(&l->namedActorNames, false);
	ListFree(&l->namedActorPointers, false);
	ListFree(&l->actors, false);
	free(l);
	l = NULL;
}

void AddActor(Actor *actor)
{
	Level *l = GetState()->level;
	ListAdd(&l->actors, actor);
}

void RemoveActor(Actor *actor)
{
	Level *l = GetState()->level;
	ActorFireOutput(actor, ACTOR_KILLED_OUTPUT, PARAM_NONE);

	// Remove the actor from the named actor lists if it's there
	const size_t nameIdx = ListFind(l->namedActorPointers, actor);
	if (nameIdx != -1)
	{
		char *name = ListGet(l->namedActorNames, nameIdx);
		free(name);
		ListRemoveAt(&l->namedActorNames, nameIdx);
		ListRemoveAt(&l->namedActorPointers, nameIdx);
	}

	const size_t idx = ListFind(l->actors, actor);
	if (idx == -1)
	{
		return;
	}
	ListRemoveAt(&l->actors, idx);
	FreeActor(actor);
}

void NameActor(Actor *actor, const char *name, Level *l)
{
	char *nameCopy = strdup(name);
	ListAdd(&l->namedActorNames, nameCopy);
	ListAdd(&l->namedActorPointers, actor);
}

Actor *GetActorByName(const char *name, const Level *l)
{
	ListLock(l->namedActorNames);
	for (int i = 0; i < l->namedActorNames.length; i++)
	{
		const char *actorName = ListGet(l->namedActorNames, i);
		if (strcmp(actorName, name) == 0)
		{
			Actor *a = ListGet(l->namedActorPointers, i);
			ListUnlock(l->namedActorNames);
			return a;
		}
	}
	ListUnlock(l->namedActorNames);
	return NULL;
}

List *GetActorsByName(const char *name, const Level *l)
{
	List *actors = malloc(sizeof(List));
	CheckAlloc(actors);
	ListCreate(actors);
	ListLock(l->namedActorNames);
	for (int i = 0; i < l->namedActorNames.length; i++)
	{
		const char *actorName = ListGet(l->namedActorNames, i);
		if (strcmp(actorName, name) == 0)
		{
			Actor *a = ListGet(l->namedActorPointers, i);
			ListAdd(actors, a);
		}
	}
	ListUnlock(l->namedActorNames);
	return actors;
}

void RenderLevel(const GlobalState *g)
{
	RenderLevel3D(g->level, g->cam);
}
