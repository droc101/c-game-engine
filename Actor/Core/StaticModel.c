//
// Created by droc101 on 5/28/25.
//

#include "StaticModel.h"
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/KVList.h"

void StaticModelInit(Actor *this, const b2WorldId /*worldId*/, const KvList *params)
{
	char modelPath[80];
	snprintf(modelPath, 80, "model/%s.gmdl", KvGetString(params, "model", "model_leafy"));
	this->actorModel = LoadModel(modelPath);
	this->yPosition = KvGetFloat(params, "yPosition", 0.0f);
	this->actorModelSkin = KvGetInt(params, "skin", 0);
}
