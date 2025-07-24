//
// Created by droc101 on 4/21/2024.
//

#include "Wall.h"
#include <math.h>
#include <string.h>
#include "../defines.h"
#include "../Helpers/Core/Error.h"

Wall *CreateWall(const Vector2 a, const Vector2 b, const char *texture, const float uvScale, const float uvOffset)
{
	Wall *w = malloc(sizeof(Wall));
	CheckAlloc(w);
	w->a = a;
	w->b = b;
	strncpy(w->tex, texture, 80);
	w->uvScale = uvScale;
	w->uvOffset = uvOffset;
	w->height = 1.0f;
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
																						  JPH_DEFAULT_CONVEX_RADIUS);
	const JPH_Shape *shape = (const JPH_Shape *)JPH_ConvexHullShapeSettings_CreateShape(shapeSettings);
	const JPH_Vec3 position = {wall->a.x, 0, wall->a.y};
	JPH_BodyCreationSettings *settings = JPH_BodyCreationSettings_Create3(
			shape,
			&position,
			NULL, // Because joltc doesn't expose JPH::Quat::sIdentity() this is what we have to do to get the identity quaternion (which is [0, 0, 0, 1])
			JPH_MotionType_Static,
			OBJECT_LAYER_STATIC);

	wall->bodyId = JPH_BodyInterface_CreateAndAddBody(bodyInterface, settings, JPH_Activation_DontActivate);
	JPH_BodyCreationSettings_Destroy(settings);
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
	w->angle = atan2f(w->b.y - w->a.y, w->b.x - w->a.x);
}
