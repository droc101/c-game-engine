//
// Created by droc101 on 4/28/25.
//

#include "Laser.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Structs/Actor.h"
#include "../Structs/Vector2.h"

typedef struct LaserData
{
	LaserHeight height;
	bool enabled;
} LaserData;

bool LaserSignalHandler(Actor *this, const Actor *sender, byte signal, const Param *param)
{
	if (DefaultSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	LaserData *data = this->extraData;
	if (signal == LASER_INPUT_DISABLE)
	{
		data->enabled = false;
		this->actorWall->b = v2(0.01, 0);
		ActorWallBake(this);
		return true;
	}
	if (signal == LASER_INPUT_ENABLE)
	{
		data->enabled = true;
		return true;
	}
	return false;
}

void LaserInit(Actor *this, const KvList *params, Transform *transform)
{
	LaserData *data = calloc(1, sizeof(LaserData));
	CheckAlloc(data);
	this->extraData = data;
	data->height = (LaserHeight)KvGetByte(params, "height", LASER_HEIGHT_MIDDLE);
	data->enabled = KvGetBool(params, "startEnabled", true);

	this->SignalHandler = LaserSignalHandler;
	this->actorWall = malloc(sizeof(ActorWall));
	this->actorWall->a = v2s(0);
	this->actorWall->b = v2s(0);
	strncpy(this->actorWall->tex,
			data->height == LASER_HEIGHT_TRIPLE ? TEXTURE("actor_triplelaser") : TEXTURE("actor_laser"),
			80);
	this->actorWall->uvScale = 1.0f;
	this->actorWall->uvOffset = 0.0f;
	this->actorWall->height = 1.0f;

	if (!data->enabled)
	{
		this->actorWall->b = v2(0.01, 0);
		ActorWallBake(this);
	}

	switch (data->height)
	{
		case LASER_HEIGHT_FLOOR:
			transform->position.y = -0.3f;
			break;
		case LASER_HEIGHT_CEILING:
			transform->position.y = 0.3f;
			break;
		case LASER_HEIGHT_MIDDLE:
		default:
			transform->position.y = 0.0f;
			break;
	}
	ActorWallBake(this);

	// TODO: Make harmful - Depends on being able to take damage
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void LaserUpdate(Actor *this, double /*delta*/)
{
	// const LaserData *data = this->extraData;
	// if (data->enabled)
	// {
	// 	Vector2 col;
	// 	Vector2 castStart = Vector2FromAngle(this->transform.rotation.y);
	// 	castStart = Vector2Scale(castStart, 0.01);
	// 	castStart = Vector2Add(castStart, this->position);
	// 	uint16_t mask = ~(COLLISION_GROUP_PLAYER | COLLISION_GROUP_HURTBOX | COLLISION_GROUP_TRIGGER);
	// 	if (data->height == LASER_HEIGHT_TRIPLE)
	// 	{
	// 		mask &= ~COLLISION_GROUP_ACTOR; // Don't hit other lasers
	// 	}
	// 	const bool rc = PerformRaycast(castStart, this->rotation, 50.0f, &col, COLLISION_GROUP_ACTOR, mask);
	// 	if (rc)
	// 	{
	// 		this->actorWall->b = Vector2Sub(col, this->position);
	// 		WallBake(this->actorWall);
	// 	}
	// 	if (GetState()->physicsFrame % 4 == 0)
	// 	{
	// 		this->actorWall->uvOffset = fmodf(this->actorWall->uvOffset + 0.5f, 1.0f);
	// 	}
	// }
}
