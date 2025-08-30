//
// Created by droc101 on 5/29/25.
//

#include "Sprite.h"
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Actor.h"
#include "../../Structs/ActorDefinition.h"
#include "../../Structs/Vector2.h"
#include "../../Structs/Wall.h"

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_SPRITE,
	.Update = DefaultActorUpdate,
	.SignalHandler = DefaultActorSignalHandler,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
};

void SpriteInit(Actor *this, const KvList *params, Transform *transform)
{
	this->definition = &definition;

	const float halfWidth = KvGetFloat(params, "width", 1.0f) * 0.5f;
	transform->position.y = KvGetFloat(params, "yPosition", 0.0f);
	ActorCreateEmptyBody(this, transform);
	this->actorWall = malloc(sizeof(ActorWall));
	this->actorWall->a = v2(halfWidth, 0.0f);
	this->actorWall->b = v2(-halfWidth, 0.0f);
	snprintf(this->actorWall->tex, 80, TEXTURE("%s"), KvGetString(params, "texture", "level/uvtest"));
	this->actorWall->uvScale = 1.0f;
	this->actorWall->uvOffset = 0.0f;
	this->actorWall->height = KvGetFloat(params, "height", 1.0f);
	ActorWallBake(this);
}
