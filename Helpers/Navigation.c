//
// Created by noah on 2/8/25.
//

#include "Navigation.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "Collision.h"
#include "Core/MathEx.h"

float PlayerRelativeAngle(const Actor *actor)
{
	const Vector2 playerPosition = GetState()->level->player.position;
	const float actorPlayerAngleDifference = atan2f(playerPosition.y - actor->position.y,
													playerPosition.x - actor->position.x);
	return wrap(actorPlayerAngleDifference - actor->rotation, -PIf, PIf) + PIf / 2;
}

bool IsPlayerVisibleInternal(const Actor *actor,
							 const NavigationConfig navigationConfig,
							 const Vector2 playerPosition,
							 const float relativeAngle,
							 const Vector2 playerRelativePosition)
{
	const float agroDistance = navigationConfig.agroTicksRemaining > 0.5 ? navigationConfig.deAgroDistance
																		 : navigationConfig.agroDistance;
	if (Vector2Distance(actor->position, playerPosition) > agroDistance)
	{
		return false;
	}
	if (fabsf(relativeAngle) > navigationConfig.fov / 2)
	{
		return false;
	}

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
	const Vector2 playerPosition = GetState()->level->player.position;
	const float actorPlayerAngleDifference = atan2f(playerPosition.y - actor->position.y,
													playerPosition.x - actor->position.x);
	const float relativeAngle = wrap(actorPlayerAngleDifference - actor->rotation, -PIf, PIf) + PIf / 2;
	const Vector2 playerRelativePosition = Vector2Sub(playerPosition, actor->position);
	return IsPlayerVisibleInternal(actor, navigationConfig, playerPosition, relativeAngle, playerRelativePosition);
}

void NavigationStep(Actor *actor, NavigationConfig *navigationConfig, const double delta)
{
	const Vector2 playerPosition = GetState()->level->player.position;
	const float actorPlayerAngleDifference = atan2f(playerPosition.y - actor->position.y,
													playerPosition.x - actor->position.x);
	const float relativeAngle = wrap(actorPlayerAngleDifference - actor->rotation, -PIf, PIf) + PIf / 2;
	const Vector2 playerRelativePosition = Vector2Sub(playerPosition, actor->position);
	if (!IsPlayerVisibleInternal(actor, *navigationConfig, playerPosition, relativeAngle, playerRelativePosition))
	{
		if (navigationConfig->agroTicksRemaining > 0.5)
		{
			const float distance = Vector2Distance(navigationConfig->lastKnownTarget, actor->position);
			if (distance < navigationConfig->minDistance || distance > navigationConfig->deAgroDistance)
			{
				navigationConfig->agroTicksRemaining = 0;
				navigationConfig->ticksUntilDirectionChange = 0;
				if (navigationConfig->IdleFunction)
				{
					navigationConfig->IdleFunction(actor, delta);
				}
				return;
			}
			navigationConfig->agroTicksRemaining -= delta;
			navigationConfig->ticksUntilDirectionChange -= delta;

			goto move;
		}
		if (navigationConfig->agroTicksRemaining != 0)
		{
			navigationConfig->agroTicksRemaining = 0;
			navigationConfig->ticksUntilDirectionChange = 0;
		}
		if (navigationConfig->IdleFunction)
		{
			navigationConfig->IdleFunction(actor, delta);
		}
		return;
	}

	navigationConfig->lastKnownTarget = playerPosition;
	if (Vector2Distance(playerPosition, actor->position) < navigationConfig->minDistance)
	{
		if (navigationConfig->TargetReachedFunction)
		{
			navigationConfig->TargetReachedFunction(actor, delta);
		} else if (navigationConfig->IdleFunction)
		{
			navigationConfig->IdleFunction(actor, delta);
		}
		return;
	}
	navigationConfig->agroTicksRemaining = navigationConfig->agroTicks;
move:
	actor->rotation += lerp(0, relativeAngle, navigationConfig->rotationSpeed * (float)delta);
	navigationConfig->ticksUntilDirectionChange -= delta;
	if (navigationConfig->ticksUntilDirectionChange < 0.5)
	{
		navigationConfig->ticksUntilDirectionChange = (double)rand() / (double)RAND_MAX * 50 + 10;
		navigationConfig->directionModifier = ((float)rand() / (float)RAND_MAX * 2 - 1) *
											  (navigationConfig->directness - 1);
	}
	const Vector2 direction = Vector2Normalize(Vector2FromAngle(actorPlayerAngleDifference +
																navigationConfig->directionModifier));
	// b2Body_ApplyLinearImpulseToCenter(actor->bodyId, Vector2Scale(direction, navigationConfig->speed * delta), true);
}
