//
// Created by droc101 on 4/21/2024.
//

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
		this->actorWall->angle = JPH_Quat_GetRotationAngle(&rotation, &Vector3_AxisY);
	} else
	{
		this->actorWall->angle = atan2f(dy, dx);
	}
}

JPH_Shape *ActorWallCreateCollider()
{
	static const Vector3 points[4] = {
		{
			0.0f,
			-0.5f,
			0.5f,
		},
		{
			0.0f,
			-0.5f,
			-0.5f,
		},
		{
			0.0f,
			0.5f,
			0.5f,
		},
		{
			0.0f,
			0.5f,
			-0.5f,
		},
	};
	return (JPH_Shape *)JPH_ConvexHullShape_Create(points, 4, JPH_DefaultConvexRadius);
}

void FreeActorWall(const ActorWall *wall)
{
	free(wall->tex);
}
