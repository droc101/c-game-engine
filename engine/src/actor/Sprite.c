//
// Created by droc101 on 5/29/25.
//

#include <engine/actor/Sprite.h>
#include <engine/assets/AssetReader.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Transform.h>
#include <stdio.h>
#include <stdlib.h>

void SpriteInit(Actor *this, const KvList params, Transform *transform)
{
	const float halfWidth = KvGetFloat(params, "width", 1.0f) * 0.5f;
	ActorCreateEmptyBody(this, transform);
	this->actorWall = malloc(sizeof(ActorWall));
	CheckAlloc(this->actorWall);
	this->actorWall->a = v2(halfWidth, 0.0f);
	this->actorWall->b = v2(-halfWidth, 0.0f);
	const char *wallTexture = KvGetString(params, "texture", "level/uvtest");
	this->actorWall->tex = malloc(strlen(TEXTURE("")) + strlen(wallTexture) + 1);
	sprintf(this->actorWall->tex, TEXTURE("%s"), wallTexture);
	this->actorWall->uvScale = KvGetFloat(params, "uvScale", 1.0f);
	this->actorWall->uvOffset = KvGetFloat(params, "uvOffset", 0.0f);
	this->actorWall->height = KvGetFloat(params, "height", 1.0f);
	this->actorWall->unshaded = KvGetBool(params, "unshaded", false);
	ActorWallBake(this);
}

ActorDefinition spriteActorDefinition = {
	.Update = DefaultActorUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = SpriteInit,
};

void RegisterSprite()
{
	RegisterDefaultActorInputs(&spriteActorDefinition);
	RegisterActor(SPRITE_ACTOR_NAME, &spriteActorDefinition);
}
