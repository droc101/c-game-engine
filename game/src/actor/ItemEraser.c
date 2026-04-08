//
// Created by droc101 on 4/8/26.
//

#include "actor/ItemEraser.h"
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Map.h>
#include <joltc/enums.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyID.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <stdbool.h>
#include "item/EraserItem.h"

static inline void CreateItemEraserCollider(Actor *this, const Transform *transform)
{
	JPH_BodyCreationSettings
			*bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->model->collisionModelShape,
																		  transform,
																		  JPH_MotionType_Dynamic,
																		  OBJECT_LAYER_DYNAMIC,
																		  this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

void ItemEraserInit(Actor *this, const KvList /*params*/, Transform *transform)
{
	this->hasModel = true;
	this->model = LoadModel(MODEL("eraser_w"));
	this->flags = ACTOR_FLAG_INTERACTABLE;

	CreateItemEraserCollider(this, transform);
}

static void ItemEraserOnPlayerContactAdded(Actor *this, JPH_BodyID /*bodyId*/)
{
	GiveItem(&eraserItemDefinition, true);
	RemoveActor(this);
}

static void ItemEraserInteracted(Actor *this)
{
	GiveItem(&eraserItemDefinition, true);
	RemoveActor(this);
}

ActorDefinition itemEraserActorDefinition = {
	.Update = DefaultActorUpdate,
	.OnPlayerContactAdded = ItemEraserOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Interact = ItemEraserInteracted,
	.Destroy = DefaultActorDestroy,
	.Init = ItemEraserInit,
};

void RegisterItemEraser()
{
	RegisterDefaultActorInputs(&itemEraserActorDefinition);
	RegisterActor(ITEM_ERASER_ACTOR_NAME, &itemEraserActorDefinition);
}
