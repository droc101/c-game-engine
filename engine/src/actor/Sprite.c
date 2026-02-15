//
// Created by droc101 on 5/29/25.
//

#include <engine/actor/Sprite.h>
#include <engine/assets/AssetReader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <joltc/enums.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Body/MassProperties.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void CreateSpriteCollider(Actor *this, const Transform *transform)
{
	JPH_Shape *shape = ActorWallCreateCollider(this->actorWall);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(shape,
																						   transform,
																						   JPH_MotionType_Kinematic,
																						   OBJECT_LAYER_DYNAMIC,
																						   this);
	const JPH_MassProperties massProperties = {
		.mass = 1.0f,
	};
	JPH_BodyCreationSettings_SetMassPropertiesOverride(bodyCreationSettings, &massProperties);
	JPH_BodyCreationSettings_SetOverrideMassProperties(bodyCreationSettings,
													   JPH_OverrideMassProperties_CalculateInertia);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_Shape_Destroy(shape);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

void SpriteInit(Actor *this, const KvList params, Transform *transform)
{
	const float halfWidth = KvGetFloat(params, "width", 1.0f) * 0.5f;
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
	if (KvGetBool(params, "solid", false))
	{
		CreateSpriteCollider(this, transform);
	} else
	{
		ActorCreateEmptyBody(this, transform);
	}
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
