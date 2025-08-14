//
// Created by droc101 on 5/28/25.
//

#include "StaticModel.h"
#include <joltc.h>
#include <stdio.h>
#include "../../Helpers/Core/AssetLoaders/ModelLoader.h"
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Actor.h"

void StaticModelInit(Actor *this, const KvList *params, Transform *transform)
{
	char modelPath[80];
	snprintf(modelPath, 80, MODEL("%s"), KvGetString(params, "model", "leafy"));
	this->actorModel = LoadModel(modelPath);
	transform->position.y = KvGetFloat(params, "yPosition", 0.0f);
	this->currentSkinIndex = KvGetInt(params, "skin", 0);
	ActorCreateEmptyBody(this, transform);
}
