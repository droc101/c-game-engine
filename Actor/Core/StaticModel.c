//
// Created by droc101 on 5/28/25.
//

#include "StaticModel.h"
#include <joltc/Math/Transform.h>
#include <stdio.h>
#include "../../Helpers/Core/AssetLoaders/ModelLoader.h"
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Actor.h"
#include "../../Structs/ActorDefinitions.h"

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

void StaticModelInit(Actor *this, const KvList *params, Transform *transform)
{
	this->definition = &definition;

	char modelPath[80];
	snprintf(modelPath, 80, MODEL("%s"), KvGetString(params, "model", "leafy"));
	this->actorModel = LoadModel(modelPath);
	transform->position.y = KvGetFloat(params, "yPosition", 0.0f);
	this->currentSkinIndex = KvGetInt(params, "skin", 0);
	ActorCreateEmptyBody(this, transform);
}
