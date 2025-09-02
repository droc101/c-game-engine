//
// Created by droc101 on 5/28/25.
//

#include "StaticModel.h"
#include <joltc/enums.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <stdio.h>
#include <wchar.h>
#include "../../Helpers/Core/AssetLoaders/ModelLoader.h"
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Helpers/Core/Physics/Physics.h"
#include "../../Structs/Actor.h"
#include "../../Structs/ActorDefinition.h"

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_STATIC_MODEL,
	.Update = DefaultActorUpdate,
	.SignalHandler = DefaultActorSignalHandler,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
};

static inline void CreateStaticModelCollider(Actor *this, const Transform *transform)
{
	JPH_BodyCreationSettings *bodyCreationSettings = NULL;
	if (this->actorModel->collisionModelType == COLLISION_MODEL_TYPE_STATIC)
	{
		bodyCreationSettings = CreateStaticModelBodyCreationSettings(transform, this->actorModel, this);
	} else if (this->actorModel->collisionModelType == COLLISION_MODEL_TYPE_DYNAMIC)
	{
		bodyCreationSettings = CreateDynamicModelBodyCreationSettings(transform,
																	  this->actorModel,
																	  JPH_MotionType_Static,
																	  OBJECT_LAYER_STATIC,
																	  this);
	} else
	{
		bodyCreationSettings = CreateBoundingBoxBodyCreationSettings(transform,
																	 this->actorModel,
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

void StaticModelInit(Actor *this, const KvList *params, Transform *transform)
{
	this->definition = &definition;

	char modelPath[80];
	snprintf(modelPath, 80, MODEL("%s"), KvGetString(params, "model", "leafy"));
	this->actorModel = LoadModel(modelPath);
	transform->position.y = KvGetFloat(params, "yPosition", 0.0f);
	this->currentSkinIndex = KvGetInt(params, "skin", 0);
	// ActorCreateEmptyBody(this, transform);
	CreateStaticModelCollider(this, transform);
}
