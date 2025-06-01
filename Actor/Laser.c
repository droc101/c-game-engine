//
// Created by droc101 on 4/28/25.
//

#include "Laser.h"

#include "../Helpers/Collision.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/KVList.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

typedef enum LaserHeight
{
	FLOOR,
	MID,
	CEILING
} LaserHeight;

typedef struct LaserData
{
	LaserHeight height;
} LaserData;

void LaserInit(Actor *this, b2WorldId, const KvList *params)
{
	this->extraData = calloc(1, sizeof(LaserData));
	this->showShadow = false;
	this->actorWall = CreateWall(v2s(0), v2s(0), TEXTURE("actor_laser"), 1.0f, 0.0f);

	LaserData *data = this->extraData;
	data->height = (LaserHeight)KvGetByte(params, "height", MID);

	if (data->height == FLOOR)
	{
		this->yPosition = -0.3f;
	} else if (data->height == CEILING)
	{
		this->yPosition = 0.3f;
	} else
	{
		this->yPosition = 0.0f;
	}
	WallBake(this->actorWall);

	// TODO: Make harmful - Depends on being able to take damage
}

void LaserUpdate(Actor *this, double)
{
	Vector2 col;
	Vector2 castStart = Vector2FromAngle(this->rotation);
	castStart = Vector2Scale(castStart, 0.01);
	castStart = Vector2Add(castStart, this->position);
	const bool rc = PerformRaycast(castStart,
								   this->rotation,
								   50.0f,
								   &col,
								   COLLISION_GROUP_ACTOR,
								   ~(COLLISION_GROUP_PLAYER | COLLISION_GROUP_HURTBOX | COLLISION_GROUP_TRIGGER));
	if (rc)
	{
		this->actorWall->b = Vector2Sub(col, this->position);
		WallBake(this->actorWall);
	}
	if (GetState()->physicsFrame % 4 == 0)
	{
		this->actorWall->uvOffset = fmod(this->actorWall->uvOffset + 0.5f, 1.0f);
	}
}

void LaserDestroy(Actor *this)
{
	free(this->actorWall);
	free(this->extraData);
}
