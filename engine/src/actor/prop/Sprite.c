//
// Created by droc101 on 5/29/25.
//

#include <engine/actor/prop/Sprite.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/Color.h>
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
#include <stdlib.h>
#include <string.h>

static inline void CreateSpriteCollider(Actor *this, const Transform *transform)
{
	JPH_Shape *shape = ActorWallCreateCollider(this->wall);
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
	const Vector2 size = KvGetVec2(params, "size", v2s(1.0f));
	this->wall = malloc(sizeof(ActorWall));
	CheckAlloc(this->wall);
	this->wall->a = v2(size.x * 0.5f, 0.0f);
	this->wall->b = v2(-size.x * 0.5f, 0.0f);
	this->wall->height = size.y;
	this->wall->tex = strdup(KvGetString(params, "texture", "level/uvtest"));
	this->wall->uvScale = KvGetVec2(params, "uv_scale", v2s(1.0f));
	this->wall->uvOffset = KvGetVec2(params, "uv_offset", v2s(0.0f));
	this->wall->unshaded = KvGetBool(params, "unshaded", false);
	this->modColor = KvGetColor(params, "color", COLOR_WHITE);
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
	.Interact = DefaultActorInteract,
	.Destroy = DefaultActorDestroy,
	.Init = SpriteInit,
};

void RegisterSprite()
{
	RegisterDefaultActorInputs(&spriteActorDefinition);
	RegisterActor(SPRITE_ACTOR_NAME, &spriteActorDefinition);
}
