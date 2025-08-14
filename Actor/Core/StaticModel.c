//
// Created by droc101 on 5/28/25.
//

#include "StaticModel.h"
#include <joltc.h>
#include <stdio.h>
#include "../../Helpers/Core/AssetLoaders/ModelLoader.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Helpers/Core/Physics/Physics.h"
#include "../../Structs/Actor.h"

static void StaticModelCreateBody(Actor *this, const Transform *transform)
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

void StaticModelInit(Actor *this, const KvList *params, Transform *transform)
{
	char modelPath[80];
	snprintf(modelPath, 80, "model/%s.gmdl", KvGetString(params, "model", "model_leafy"));
	this->actorModel = LoadModel(modelPath);
	transform->position.y = KvGetFloat(params, "yPosition", 0.0f);
	this->currentSkinIndex = KvGetInt(params, "skin", 0);

	StaticModelCreateBody(this, transform);
}
