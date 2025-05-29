//
// Created by droc101 on 5/28/25.
//

#include "StaticModel.h"

#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/KVList.h"

void StaticModelInit(Actor *this, const b2WorldId, KvList *params)
{
	this->actorModel = LoadModel(KvGetString(params, "model", MODEL("leafy")));
	this->shadowSize = KvGetFloat(params, "shadowSize", 1.0f);
	this->showShadow = KvGetBool(params, "showShadow", true);
	this->yPosition = KvGetFloat(params, "yPosition", 0.0f);
	this->actorModelSkin = KvGetInt(params, "skin", 0);
}
