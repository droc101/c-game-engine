//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_WALL_H
#define GAME_WALL_H

#include <engine/structs/Camera.h>
#include <engine/structs/Vector2.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stdbool.h>

typedef struct Actor Actor;

typedef enum ActorWallOrientation ActorWallOrientation;

typedef struct ActorWall ActorWall;

enum ActorWallOrientation
{
	/// Wall should be along the actor's local X axis
	X_AXIS,
	/// Wall should be along the actor's local Z axis
	Z_AXIS,
};

struct ActorWall
{
	/// Which axis the wall extends along
	ActorWallOrientation orientation;
	/// The local center of the wall
	Vector2 localCenter;
	/// The fully qualified texture name (texture/level/uvtest.gtex instead of level/uvtest)
	char *tex;
	/// The UV scale of the wall
	Vector2 uvScale;
	/// The UV offset of the wall
	Vector2 uvOffset;
	/// height of the wall for rendering. Does not affect collision
	float height;
	/// The length of the wallS
	float length;
	/// Whether the wall should be rendered without shading
	bool unshaded;
};

/**
 * Create a collider for an actor wall
 * @return The shape for the collider
 */
JPH_Shape *ActorWallCreateCollider(const ActorWall *wall);

/**
 * Free and actor wall
 * @param wall The wall to free
 * @note This does NOT free the wall pointer itself
 */
void FreeActorWall(const ActorWall *wall);

void ActorYBillboard(Camera *camera, Actor *this);

#endif //GAME_WALL_H
