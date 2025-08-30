//
// Created by droc101 on 4/21/2024.
//

#include "Wall.h"
#include <joltc/constants.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Physics/Physics.h"
#include "Vector2.h"

Wall *CreateWall(const Vector2 a, const Vector2 b, const char *texture, const float uvScale, const float uvOffset)
{
	Wall *w = malloc(sizeof(Wall));
	CheckAlloc(w);
	w->a = a;
	w->b = b;
	strncpy(w->tex, texture, 80);
	w->uvScale = uvScale;
	w->uvOffset = uvOffset;
	w->bodyId = -1;
	return w;
}

void CreateWallCollider(Wall *wall, JPH_BodyInterface *bodyInterface)
{
	const Vector3 points[4] = {
		{
			0,
			-0.5f,
			0,
		},
		{
			wall->dx,
			-0.5f,
			wall->dy,
		},
		{
			0,
			0.5f,
			0,
		},
		{
			wall->dx,
			0.5f,
			wall->dy,
		},
	};
	const JPH_ConvexHullShapeSettings *shapeSettings = JPH_ConvexHullShapeSettings_Create(points,
																						  4,
																						  JPH_DefaultConvexRadius);
	const JPH_Shape *shape = (const JPH_Shape *)JPH_ConvexHullShapeSettings_CreateShape(shapeSettings);
	const Vector3 position = {wall->a.x, 0, wall->a.y};
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create3(shape,
																					  &position,
																					  &JPH_Quat_Identity,
																					  JPH_MotionType_Static,
																					  OBJECT_LAYER_STATIC);

	wall->bodyId = JPH_BodyInterface_CreateAndAddBody(bodyInterface, bodyCreationSettings, JPH_Activation_DontActivate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
	JPH_ShapeSettings_Destroy((JPH_ShapeSettings *)shapeSettings);
}

void FreeWall(JPH_BodyInterface *bodyInterface, Wall *wall)
{
	JPH_BodyInterface_RemoveAndDestroyBody(bodyInterface, wall->bodyId);
	free(wall);
}

void WallBake(Wall *w)
{
	w->dx = w->b.x - w->a.x;
	w->dy = w->b.y - w->a.y;
	w->length = sqrtf(w->dx * w->dx + w->dy * w->dy);
	w->angle = atan2f(w->b.x - w->a.x, w->b.y - w->a.y);
}

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

const JPH_Shape *ActorWallCreateCollider()
{
	static const Vector3 points[4] = {
		{
			0.0f,
			-0.5f,
			0.0f,
		},
		{
			0.0f,
			-0.5f,
			-1.0f,
		},
		{
			0.0f,
			0.5f,
			0.0f,
		},
		{
			0.0f,
			0.5f,
			-1.0f,
		},
	};
	return (const JPH_Shape *)JPH_ConvexHullShape_Create(points, 4, JPH_DefaultConvexRadius);
}
