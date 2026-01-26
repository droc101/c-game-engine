//
// Created by droc101 on 4/21/2024.
//

#include <cglm/types.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/Vector2.h>
#include <joltc/constants.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

void ActorWallBake(const Actor *this)
{
	const float dx = this->actorWall->b.x - this->actorWall->a.x;
	const float dy = this->actorWall->b.y - this->actorWall->a.y;
	this->actorWall->length = sqrtf(dx * dx + dy * dy);
	if (this->bodyId != JPH_BodyId_InvalidBodyID && this->bodyInterface != NULL)
	{
		JPH_Quat rotation = {};
		JPH_BodyInterface_GetRotation(this->bodyInterface, this->bodyId, &rotation);
		this->actorWall->angle = JPH_Quat_GetRotationAngle(&rotation, &Vector3_AxisY) + atan2f(dy, dx);
	} else
	{
		this->actorWall->angle = atan2f(dy, dx);
	}
	this->actorWall->angle += GLM_PI_2f;
}

JPH_Shape *ActorWallCreateCollider(const ActorWall *wall)
{
	const Vector3 points[4] = {
		{
			wall->a.x,
			-(wall->height / 2),
			wall->a.y,
		},
		{
			wall->b.x,
			-(wall->height / 2),
			wall->b.y,
		},
		{
			wall->a.x,
			(wall->height / 2),
			wall->a.y,
		},
		{
			wall->b.x,
			(wall->height / 2),
			wall->b.y,
		},
	};
	return (JPH_Shape *)JPH_ConvexHullShape_Create(points, 4, JPH_DefaultConvexRadius);
}

void FreeActorWall(const ActorWall *wall)
{
	free(wall->tex);
}
