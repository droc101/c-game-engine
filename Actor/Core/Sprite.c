//
// Created by droc101 on 5/29/25.
//

#include "Sprite.h"
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Actor.h"
#include "../../Structs/Vector2.h"

void SpriteCreateBody(Actor *this, const Transform *transform)
{
	const JPH_ShapeSettings *shapeSettings = (JPH_ShapeSettings *)JPH_EmptyShapeSettings_Create(&Vector3_Zero);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create_GAME(shapeSettings,
																						  transform,
																						  JPH_MotionType_Static,
																						  OBJECT_LAYER_STATIC,
																						  this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_DontActivate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

void SpriteInit(Actor *this, const KvList *params, Transform *transform)
{
	const float halfWidth = KvGetFloat(params, "width", 1.0f) * 0.5f;
	transform->position.y = KvGetFloat(params, "yPosition", 0.0f);

	SpriteCreateBody(this, transform);

	this->actorWall = malloc(sizeof(ActorWall));
	this->actorWall->a = v2(halfWidth, 0.0f);
	this->actorWall->b = v2(-halfWidth, 0.0f);
	snprintf(this->actorWall->tex, 80, "texture/%s.gtex", KvGetString(params, "texture", "level_uvtest"));
	this->actorWall->uvScale = 1.0f;
	this->actorWall->uvOffset = 0.0f;
	this->actorWall->height = KvGetFloat(params, "height", 1.0f);
	ActorWallBake(this);
}
