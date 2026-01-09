//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_WALL_H
#define GAME_WALL_H

#include <engine/structs/Vector2.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stdbool.h>

typedef struct Actor Actor;

typedef struct ActorWall ActorWall;

struct ActorWall
{
	/// The first point of the wall
	Vector2 a;
	/// The second point of the wall
	Vector2 b;
	/// The fully qualified texture name (texture/level/uvtest.gtex instead of level/uvtest)
	char *tex;
	/// The X scale of the texture
	float uvScale;
	/// The X offset of the texture
	float uvOffset;
	/// height of the wall for rendering. Does not affect collision
	float height;
	/// The length of the wall (Call @c WallBake to update)
	float length;
	/// The angle of the wall (Call @c WallBake to update)
	float angle;
	/// Whether the wall should be rendered without shading
	bool unshaded;
};

/**
 * Bake an actor wall's information
 * @param this ActorWall to bake
 */
void ActorWallBake(const Actor *this);

/**
 * Create a collider for an actor wall
 * @return The shape for the collider
 */
JPH_Shape *ActorWallCreateCollider();

/**
 * Free and actor wall
 * @param wall The wall to free
 * @note This does NOT free the wall pointer itself
 */
void FreeActorWall(const ActorWall *wall);

#endif //GAME_WALL_H
