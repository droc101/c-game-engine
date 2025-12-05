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
#include <joltc/enums.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <stddef.h>
#include <stdio.h>

static inline void CreateStaticModelCollider(Actor *this, const Transform *transform)
{
	JPH_BodyCreationSettings *bodyCreationSettings = NULL;
	if (this->actorModel->collisionModelType == COLLISION_MODEL_TYPE_STATIC ||
		this->actorModel->collisionModelType == COLLISION_MODEL_TYPE_DYNAMIC)
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
	char modelPath[80];
	snprintf(modelPath, 80, MODEL("%s"), KvGetString(params, "model", "leafy"));
	this->actorModel = LoadModel(modelPath);
	transform->position.y = KvGetFloat(params, "yPosition", 0.0f);
	this->currentSkinIndex = KvGetInt(params, "skin", 0);
	this->modColor = KvGetColor(params, "color", COLOR_WHITE);
	// ActorCreateEmptyBody(this, transform);
	CreateStaticModelCollider(this, transform);
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_STATIC_MODEL,
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
	RegisterDefaultActorInputs(&definition);
	RegisterActor(STATIC_MODEL_ACTOR_NAME, &definition);
}
