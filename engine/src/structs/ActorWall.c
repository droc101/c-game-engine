//
// Created by droc101 on 4/21/2024.
//

#include <cglm/types.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/Camera.h>
#include <joltc/constants.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

JPH_Shape *ActorWallCreateCollider(const ActorWall *wall)
{
	const float halfLength = wall->length / 2.0f;
	const float halfHeight = wall->height / 2.0f;
	const float xComponent = (wall->orientation == X_AXIS ? halfLength : 0) + wall->localCenter.x;
	const float zComponent = (wall->orientation == Z_AXIS ? halfLength : 0) + wall->localCenter.x;
	const Vector3 points[4] = {
		{
			-xComponent,
			-halfHeight + wall->localCenter.y,
			-zComponent,
		},
		{
			xComponent,
			-halfHeight + wall->localCenter.y,
			zComponent,
		},
		{
			-xComponent,
			halfHeight + wall->localCenter.y,
			-zComponent,
		},
		{
			xComponent,
			halfHeight + wall->localCenter.y,
			zComponent,
		},
	};
	return (JPH_Shape *)JPH_ConvexHullShape_Create(points, 4, JPH_DefaultConvexRadius);
}

void FreeActorWall(const ActorWall *wall)
{
	free(wall->tex);
}

void ActorYBillboard(Camera *camera, Actor *this)
{
	assert(!this->hasModel);
	// TODO quaternion
	Vector3 position;
	JPH_BodyInterface_GetPosition(this->bodyInterface, this->bodyId, &position);
	float yaw = atan2f(camera->transform.position.x - position.x, camera->transform.position.z - position.z);
	if (this->wall->orientation == Z_AXIS)
	{
		yaw += GLM_PI_2f;
	}
	const Vector3 euler = {0, yaw, 0};
	JPH_Quat quat;
	JPH_Quat_FromEulerAngles(&euler, &quat);
	JPH_BodyInterface_SetRotation(this->bodyInterface, this->bodyId, &quat, JPH_Activation_DontActivate);
}
