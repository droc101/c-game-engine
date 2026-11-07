//
// Created by droc101 on 11/7/26.
//

#include <engine/actor/prop/DynamicModel.h>
#include <engine/assets/ModelLoader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Color.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Logging.h>
#include <joltc/enums.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline void CreateDynamicModelCollider(Actor *this,
											  const Transform *transform,
											  const bool useAABB)
{
	// TODO: Created with JPH_MotionType_Static because I don't want it moving on its own, but it will need to be able to move via parenting system once that is added.

	JPH_BodyCreationSettings *bodyCreationSettings = NULL;
	if (!useAABB)
	{
		bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->model->collisionModelShape,
																	 transform,
																	 JPH_MotionType_Static,
																	 OBJECT_LAYER_DYNAMIC,
																	 this);
	} else
	{
		bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->model->boundingBoxShape,
																	 transform,
																	 JPH_MotionType_Static,
																	 OBJECT_LAYER_DYNAMIC,
																	 this);
	}

	JPH_BodyCreationSettings_SetAllowedDOFs(bodyCreationSettings, 0);

	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

static void DynamicModelInit(Actor *this, const KvList params, const Transform *transform)
{
	this->hasModel = true;
	this->model = LoadModel(KvGetString(params, "model", "leafy"));
	this->currentSkinIndex = KvGetInt(params, "skin", 0);
	this->modColor = KvGetColor(params, "color", COLOR_WHITE);
	if (this->model->collisionModelType != COLLISION_MODEL_TYPE_DYNAMIC)
	{
		LogWarning("Tried to create a " DYNAMIC_MODEL_ACTOR_NAME
				   " with full collision, but the model file (\"%s\") does not have any!\n",
				   KvGetString(params, "model", "model/quad.gmdl"));
		CreateDynamicModelCollider(this, transform, true);
	} else
	{
		CreateDynamicModelCollider(this, transform, false);
	}
}

static void DynamicModelSetColorHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	if (param->type == PARAM_TYPE_COLOR)
	{
		this->modColor = param->colorValue;
	}
}

static void DynamicModelSetSkinHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	uint32_t newSkin = this->currentSkinIndex;
	if (param->type == PARAM_TYPE_BYTE)
	{
		newSkin = param->byteValue;
	} else if (param->type == PARAM_TYPE_INTEGER)
	{
		newSkin = param->intValue;
	} else if (param->type == PARAM_TYPE_UINT_64)
	{
		newSkin = param->uint64value;
	}
	if (this->model->skinCount <= newSkin)
	{
		LogError("Tried to switch to skin %u, but that index is out of bounds\n", newSkin);
	} else
	{
		this->currentSkinIndex = newSkin;
	}
}

ActorDefinition dynamicModelActorDefinition = {
	.Init = DynamicModelInit,
};

void RegisterDynamicModel()
{
	RegisterDefaultActorInputs(&dynamicModelActorDefinition);
	RegisterActorInput(&dynamicModelActorDefinition, DYNAMIC_MODEL_INPUT_SET_COLOR, DynamicModelSetColorHandler);
	RegisterActorInput(&dynamicModelActorDefinition, DYNAMIC_MODEL_INPUT_SET_SKIN, DynamicModelSetSkinHandler);
	RegisterActor(DYNAMIC_MODEL_ACTOR_NAME, &dynamicModelActorDefinition);
}
