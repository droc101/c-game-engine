//
// Created by droc101 on 4/21/2024.
//

#include "Wall.h"
#include <box2d/box2d.h>
#include <box2d/types.h>
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

void CreateWallCollider(Wall *wall, const b2WorldId worldId, JPH_BodyInterface *bodyInterface)
{
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_staticBody;
	bodyDef.position = wall->a;
	wall->box2dBodyId = b2CreateBody(worldId, &bodyDef);
	if (wall->dx != 0 || wall->dy != 0)
	{
		const b2Segment shape = {
			.point2 = {wall->dx, wall->dy},
		};
		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.friction = 0;
		b2CreateSegmentShape(wall->box2dBodyId, &shapeDef, &shape);
	}


	const float dx = wall->dx;
	const float dy = wall->dy;
	const float thickness = 0.01f;
	const Vector3 points[8] = {
		{
			(dy - dx / 2) * thickness / wall->length,
			-0.5f,
			(-dx - dy / 2) * thickness / wall->length,
		},
		{
			(-dy - dx / 2) * thickness / wall->length,
			-0.5f,
			(dx - dy / 2) * thickness / wall->length,
		},
		{
			dx + (dy + dx / 2) * thickness / wall->length,
			-0.5f,
			dy + (-dx + dy / 2) * thickness / wall->length,
		},
		{
			dx + (-dy + dx / 2) * thickness / wall->length,
			-0.5f,
			dy + (dx + dy / 2) * thickness / wall->length,
		},
		{
			(dy - dx / 2) * thickness / wall->length,
			0.5f,
			(-dx - dy / 2) * thickness / wall->length,
		},
		{
			(-dy - dx / 2) * thickness / wall->length,
			0.5f,
			(dx - dy / 2) * thickness / wall->length,
		},
		{
			dx + (dy + dx / 2) * thickness / wall->length,
			0.5f,
			dy + (-dx + dy / 2) * thickness / wall->length,
		},
		{
			dx + (-dy + dx / 2) * thickness / wall->length,
			0.5f,
			dy + (dx + dy / 2) * thickness / wall->length,
		},
	};
	const JPH_ConvexHullShapeSettings *shapeSettings = JPH_ConvexHullShapeSettings_Create(points,
																						  8,
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
