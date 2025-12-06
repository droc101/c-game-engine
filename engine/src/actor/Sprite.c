//
// Created by droc101 on 5/29/25.
//

#include <engine/actor/Sprite.h>
#include <engine/assets/AssetReader.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Vector2.h>
#include <engine/structs/Wall.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <stdio.h>
#include <stdlib.h>

void SpriteInit(Actor *this, const KvList params, Transform *transform)
{
	const float halfWidth = KvGetFloat(params, "width", 1.0f) * 0.5f;
	ActorCreateEmptyBody(this, transform);
	this->actorWall = malloc(sizeof(ActorWall));
	this->actorWall->a = v2(halfWidth, 0.0f);
	this->actorWall->b = v2(-halfWidth, 0.0f);
	snprintf(this->actorWall->tex, 80, TEXTURE("%s"), KvGetString(params, "texture", "level/uvtest"));
	this->actorWall->uvScale = KvGetFloat(params, "uvScale", 1.0f);
	this->actorWall->uvOffset = KvGetFloat(params, "uvOffset", 1.0f);
	this->actorWall->height = KvGetFloat(params, "height", 1.0f);
	ActorWallBake(this);
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_SPRITE,
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
	RegisterDefaultActorInputs(&definition);
	RegisterActor(SPRITE_ACTOR_NAME, &definition);
}
