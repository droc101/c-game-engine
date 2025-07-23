//
// Created by noah on 2/10/25.
//

#include "Collision.h"
#include <box2d/box2d.h>
#include <box2d/types.h>

#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "Core/Logging.h"

float RaycastCallback(const b2ShapeId shapeId, Vector2, Vector2, const float fraction, void *raycastHit)
{
	if (!raycastHit)
	{
		LogError("raycastHit was NULL, likely box2d issue");
		return -1;
	}
	*(b2ShapeId *)raycastHit = shapeId;
	return fraction;
}

float RaycastCallback_GetPosition(const b2ShapeId, const Vector2 point, Vector2, const float fraction, void *raycastHit)
{
	if (!raycastHit)
	{
		LogError("raycastHit was NULL, likely box2d issue");
		return -1;
	}
	*(Vector2 *)raycastHit = point;
	return fraction;
}

bool GetSensorState(const b2WorldId worldId, const uint sensorShapeIdIndex, const bool currentState)
{
	const b2SensorEvents sensorEvents = b2World_GetSensorEvents(worldId);
	if (currentState)
	{
		for (int i = 0; i < sensorEvents.endCount; i++)
		{
			const b2SensorEndTouchEvent event = sensorEvents.endEvents[i];
			if (event.sensorShapeId.index1 == sensorShapeIdIndex)
			{
				return false;
			}
		}
	} else
	{
		for (int i = 0; i < sensorEvents.beginCount; i++)
		{
			const b2SensorBeginTouchEvent event = sensorEvents.beginEvents[i];
			if (event.sensorShapeId.index1 == sensorShapeIdIndex)
			{
				return true;
			}
		}
	}

	return currentState;
}

Actor *GetTargetedEnemy(const float maxDistance)
{
	const GlobalState *state = GetState();
	Vector2 rayEnd = Vector2FromAngle(state->level->player.angle);
	rayEnd = Vector2Scale(rayEnd, maxDistance);
	b2ShapeId raycastHit = b2_nullShapeId;
	b2World_CastRay(state->level->worldId,
					state->level->player.pos,
					rayEnd,
					(b2QueryFilter){.categoryBits = COLLISION_GROUP_PLAYER, .maskBits = COLLISION_GROUP_HURTBOX},
					RaycastCallback,
					&raycastHit);

	if (b2Shape_IsValid(raycastHit))
	{
		ListLock(state->level->actors);
		for (int i = 0; i < state->level->actors.length; i++)
		{
			Actor *actor = ListGetPointer(state->level->actors, i);
			if (b2Shape_GetBody(raycastHit).index1 == actor->bodyId.index1)
			{
				ListUnlock(state->level->actors);
				return actor;
			}
		}
		ListUnlock(state->level->actors);
	}
	return NULL;
}

bool PerformRaycast(const Vector2 origin,
					const float angle,
					const float maxDistance,
					Vector2 *collisionPoint,
					const uint64_t category,
					const uint16_t mask)
{
	const GlobalState *state = GetState();
	Vector2 rayEnd = Vector2FromAngle(angle);
	rayEnd = Vector2Scale(rayEnd, maxDistance);
	Vector2 raycastHit = v2s(FP_NAN);
	b2World_CastRay(state->level->worldId,
					origin,
					rayEnd,
					(b2QueryFilter){.categoryBits = category, .maskBits = mask},
					RaycastCallback_GetPosition,
					&raycastHit);

	if (raycastHit.x != FP_NAN)
	{
		*collisionPoint = raycastHit;
		return true;
	}
	return false;
}
