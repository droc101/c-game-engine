//
// Created by droc101 on 5/28/25.
//

#include <engine/actor/StaticModel.h>
#include <engine/assets/AssetReader.h>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void CreateStaticModelCollider(Actor *this, const Transform *transform, const bool useAABB)
{
	JPH_BodyCreationSettings *bodyCreationSettings = NULL;
	if (!useAABB)
	{
		bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->actorModel->collisionModelShape,
																	 transform,
																	 JPH_MotionType_Static,
																	 OBJECT_LAYER_STATIC,
																	 this);
	} else
	{
		bodyCreationSettings = JPH_BodyCreationSettings_Create2_GAME(this->actorModel->boundingBoxShape,
																	 transform,
																	 JPH_MotionType_Static,
																	 OBJECT_LAYER_STATIC,
																	 this);
		this->actorFlags = ACTOR_FLAG_USING_BOUNDING_BOX_COLLISION;
	}
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

void StaticModelInit(Actor *this, const KvList params, Transform *transform)
{
	const char *modelParam = KvGetString(params, "model", "leafy");
	char *modelPath = malloc(strlen(MODEL("")) + strlen(modelParam) + 1);
	sprintf(modelPath, MODEL("%s"), modelParam);
	this->actorModel = LoadModel(modelPath);
	free(modelPath);
	this->currentSkinIndex = KvGetInt(params, "skin", 0);
	this->modColor = KvGetColor(params, "color", COLOR_WHITE);
	uint8_t collisionType = KvGetByte(params, "collision", 2);
	if (collisionType == 2 && this->actorModel->collisionModelType == COLLISION_MODEL_TYPE_NONE)
	{
		LogWarning("Tried to create a " STATIC_MODEL_ACTOR_NAME
				   " with full collision, but the model file (\"%s\") does not have any!\n",
				   KvGetString(params, "model", "leafy"));
		collisionType = 0;
	}
	switch (collisionType)
	{
		case 0:
		default:
			ActorCreateEmptyBody(this, transform);
			break;
		case 1:
			CreateStaticModelCollider(this, transform, true);
			break;
		case 2:
			CreateStaticModelCollider(this, transform, false);
			break;
	}
}

ActorDefinition staticModelActorDefinition = {
	.Update = DefaultActorUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = StaticModelInit,
};

void RegisterStaticModel()
{
	RegisterDefaultActorInputs(&staticModelActorDefinition);
	RegisterActor(STATIC_MODEL_ACTOR_NAME, &staticModelActorDefinition);
}
