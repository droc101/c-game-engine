//
// Created by droc101 on 5/29/25.
//

#include "Sprite.h"
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Vector2.h"
#include "../../Structs/Wall.h"

void SpriteInit(Actor *this, const b2WorldId /*worldId*/, const KvList *params)
{
	const float width = KvGetFloat(params, "width", 1.0f) * 0.5f;
	const float height = KvGetFloat(params, "height", 1.0f);
	char texturePath[80];
	snprintf(texturePath, 80, "texture/%s.gtex", KvGetString(params, "texture", "level_uvtest"));
	this->actorWall = CreateWall(v2s(0.0f), v2s(0.0f), texturePath, 1.0f, 0.0f);
	this->actorWall->a = v2(width * cosf(this->rotation), width * sinf(this->rotation));
	this->actorWall->b = v2(-width * cosf(this->rotation), -width * sinf(this->rotation));
	this->actorWall->height = height;
	this->shadowSize = KvGetFloat(params, "shadowSize", 1.0f);
	this->showShadow = KvGetBool(params, "showShadow", true);
	this->yPosition = KvGetFloat(params, "yPosition", 0.0f);
	WallBake(this->actorWall);
}

void SpriteDestroy(Actor *this)
{
	if (this->actorWall != NULL)
	{
		free(this->actorWall);
		this->actorWall = NULL;
	}
}
