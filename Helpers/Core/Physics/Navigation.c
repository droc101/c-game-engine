//
// Created by noah on 2/8/25.
//

#include "Navigation.h"

float PlayerRelativeAngle(const Actor *actor)
{
	// const float actorPlayerAngleDifference = atan2f(GetState()->level->player.transform.position.z - actor->transform.position.z,
	// 												GetState()->level->player.transform.position.x - actor->transform.position.x);
	// return wrap(actorPlayerAngleDifference - actor->transform.rotation.y, -PIf, PIf) + PIf / 2;
	return 0.0f;
}

bool IsPlayerVisibleInternal(const Actor *actor,
							 const NavigationConfig navigationConfig,
							 const Vector3 *playerPosition,
							 const float relativeAngle,
							 const Vector3 *playerRelativePosition)
{
	// const float agroDistance = navigationConfig.agroTicksRemaining > 0.5 ? navigationConfig.deAgroDistance
	// 																	 : navigationConfig.agroDistance;
	// if (Vector2Distance(actor->transform.position, playerPosition) > agroDistance)
	// {
	// 	return false;
	// }
	// if (fabsf(relativeAngle) > navigationConfig.fov / 2)
	// {
	// 	return false;
	// }

	return false;

	// b2ShapeId raycastHit = b2_nullShapeId;
	// b2World_CastRay(GetState()->level->worldId,
	// 				actor->position,
	// 				playerRelativePosition,
	// 				(b2QueryFilter){.categoryBits = COLLISION_GROUP_ACTOR, .maskBits = ~COLLISION_GROUP_TRIGGER},
	// 				RaycastCallback,
	// 				&raycastHit);
	//
	// return b2Shape_IsValid(raycastHit) && b2Shape_GetFilter(raycastHit).categoryBits & COLLISION_GROUP_PLAYER;
}

bool IsPlayerVisible(const Actor *actor, const NavigationConfig navigationConfig)
{
	// const float relativeAngle = PlayerRelativeAngle(actor);
	// const Vector2 playerRelativePosition = Vector2Sub(GetState()->level->player.transform.position, actor->transform.position);
	// return IsPlayerVisibleInternal(actor, navigationConfig, GetState()->level->player.transform.position, relativeAngle, playerRelativePosition);
	return false;
}

void NavigationStep(Actor *actor, NavigationConfig *navigationConfig, const double delta)
{
// 	const Vector2 playerPosition = GetState()->level->player.transform;
// 	const float actorPlayerAngleDifference = atan2f(playerPosition.y - actor->transform.y,
// 													playerPosition.x - actor->transform.x);
// 	const float relativeAngle = wrap(actorPlayerAngleDifference - actor->rotation, -PIf, PIf) + PIf / 2;
// 	const Vector2 playerRelativePosition = Vector2Sub(playerPosition, actor->position);
// 	if (!IsPlayerVisibleInternal(actor, *navigationConfig, playerPosition, relativeAngle, playerRelativePosition))
// 	{
// 		if (navigationConfig->agroTicksRemaining > 0.5)
// 		{
// 			const float distance = Vector2Distance(navigationConfig->lastKnownTarget, actor->transform);
// 			if (distance < navigationConfig->minDistance || distance > navigationConfig->deAgroDistance)
// 			{
// 				navigationConfig->agroTicksRemaining = 0;
// 				navigationConfig->ticksUntilDirectionChange = 0;
// 				if (navigationConfig->IdleFunction)
// 				{
// 					navigationConfig->IdleFunction(actor, delta);
// 				}
// 				return;
// 			}
// 			navigationConfig->agroTicksRemaining -= delta;
// 			navigationConfig->ticksUntilDirectionChange -= delta;
//
// 			goto move;
// 		}
// 		if (navigationConfig->agroTicksRemaining != 0)
// 		{
// 			navigationConfig->agroTicksRemaining = 0;
// 			navigationConfig->ticksUntilDirectionChange = 0;
// 		}
// 		if (navigationConfig->IdleFunction)
// 		{
// 			navigationConfig->IdleFunction(actor, delta);
// 		}
// 		return;
// 	}
//
// 	navigationConfig->lastKnownTarget = playerPosition;
// 	if (Vector2Distance(playerPosition, actor->transform) < navigationConfig->minDistance)
// 	{
// 		if (navigationConfig->TargetReachedFunction)
// 		{
// 			navigationConfig->TargetReachedFunction(actor, delta);
// 		} else if (navigationConfig->IdleFunction)
// 		{
// 			navigationConfig->IdleFunction(actor, delta);
// 		}
// 		return;
// 	}
// 	navigationConfig->agroTicksRemaining = navigationConfig->agroTicks;
// move:
// 	actor->rotation += lerp(0, relativeAngle, navigationConfig->rotationSpeed * (float)delta);
// 	navigationConfig->ticksUntilDirectionChange -= delta;
// 	if (navigationConfig->ticksUntilDirectionChange < 0.5)
// 	{
// 		navigationConfig->ticksUntilDirectionChange = (double)rand() / (double)RAND_MAX * 50 + 10;
// 		navigationConfig->directionModifier = ((float)rand() / (float)RAND_MAX * 2 - 1) *
// 											  (navigationConfig->directness - 1);
// 	}
// 	const Vector2 direction = Vector2Normalize(Vector2FromAngle(actorPlayerAngleDifference +
// 																navigationConfig->directionModifier));
}
