//
// Created by droc101 on 9/19/26.
//

#include "actor/item/ItemAxisHelper.h"
#include <engine/assets/AssetReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Map.h>
#include <engine/subsystem/Error.h>
#include <joltc/enums.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyID.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <stdbool.h>
#include <stdlib.h>
#include "item/AxisHelperItem.h"

typedef struct ItemAxisHelperData ItemAxisHelperData;

struct ItemAxisHelperData
{
	bool alwaysGive;
};

static inline void CreateItemAxisHelperCollider(Actor *this, const Transform *transform)
{
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->model
																								   ->boundingBoxShape,
																						   transform,
																						   JPH_MotionType_Dynamic,
																						   OBJECT_LAYER_DYNAMIC,
																						   this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

static void ItemAxisHelperInit(Actor *this, const KvList params, const Transform *transform)
{
	this->hasModel = true;
	this->model = LoadModel(MODEL("axis_helper"));
	this->flags = ACTOR_FLAG_INTERACTABLE;
	ItemAxisHelperData *data = malloc(sizeof(ItemAxisHelperData));
	CheckAlloc(data);
	data->alwaysGive = KvGetBool(params, "always_give", false);
	this->extraData = data;

	CreateItemAxisHelperCollider(this, transform);
}

static void ItemEraserActivate(Actor *this)
{
	const ItemAxisHelperData *data = this->extraData;
	if (HasItem(&AXIS_HELPER_ITEM_DEFINITION) && !data->alwaysGive)
	{
		SwitchToItem(&AXIS_HELPER_ITEM_DEFINITION);
	} else
	{
		GiveItem(&AXIS_HELPER_ITEM_DEFINITION, true);
		RemoveActor(this);
	}
}

static void ItemEraserOnPlayerContactAdded(Actor *this, JPH_BodyID /*bodyId*/)
{
	ItemEraserActivate(this);
}

static void ItemEraserInteracted(Actor *this)
{
	ItemEraserActivate(this);
}

ActorDefinition itemAxisHelperActorDefinition = {
	.OnPlayerContactAdded = ItemEraserOnPlayerContactAdded,
	.Interact = ItemEraserInteracted,
	.Init = ItemAxisHelperInit,
};

void RegisterItemAxisHelper()
{
	RegisterDefaultActorInputs(&itemAxisHelperActorDefinition);
	RegisterActor(ITEM_AXIS_HELPER_ACTOR_NAME, &itemAxisHelperActorDefinition);
}
