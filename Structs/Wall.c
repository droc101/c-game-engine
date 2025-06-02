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
	return w;
}

void CreateWallCollider(Wall *wall, const b2WorldId worldId)
{
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_staticBody;
	bodyDef.position = wall->a;
	wall->bodyId = b2CreateBody(worldId, &bodyDef);
	if (wall->dx != 0 || wall->dy != 0)
	{
		const b2Segment shape = {
			.point2 = {wall->dx, wall->dy},
		};
		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.friction = 0;
		b2CreateSegmentShape(wall->bodyId, &shapeDef, &shape);
	}
}

void WallBake(Wall *w)
{
	w->dx = w->b.x - w->a.x;
	w->dy = w->b.y - w->a.y;
	w->length = sqrtf(w->dx * w->dx + w->dy * w->dy);
	w->angle = atan2f(w->b.y - w->a.y, w->b.x - w->a.x);
}
