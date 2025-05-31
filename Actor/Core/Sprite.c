//
// Created by droc101 on 5/29/25.
//

#include "Sprite.h"
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Wall.h"
#include "../../Structs/Vector2.h"

void SpriteInit(Actor *this, const b2WorldId, KvList *params)
{
	char modelPath[64];
	snprintf(modelPath, 64, "model/%s.gmdl", KvGetString(params, "texture", "model_leafy"));

	float width = KvGetFloat(params, "width", 1.0f) * 0.5f;
	float height = KvGetFloat(params, "height", 1.0f);
	char texturePath[64];
	snprintf(texturePath, 64, "texture/%s.gtex", KvGetString(params, "texture", "level_uvtest"));
	this->actorWall = CreateWall(v2s(0.0f), v2s(0.0f), texturePath, 1.0f, 0.0f);
	this->actorWall->a = v2(width * cosf(this->rotation), width * sinf(this->rotation));
	this->actorWall->b = v2(-width * cosf(this->rotation), -width * sinf(this->rotation));
	this->actorWall->height = height;
	this->shadowSize = KvGetFloat(params, "shadowSize", 1.0f);
	this->showShadow = KvGetBool(params, "showShadow", true);
	this->yPosition = KvGetFloat(params, "yPosition", 0.0f);
	WallBake(this->actorWall);
	asm("nop");
}

void SpriteDestroy(Actor *this)
{
	if (this->actorWall != NULL)
	{
		free(this->actorWall);
		this->actorWall = NULL;
	}
}
