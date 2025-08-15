//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_WALL_H
#define GAME_WALL_H

#include <joltc/joltc.h>
#include "Vector2.h"

typedef struct Actor Actor;

typedef struct Wall Wall;
typedef struct ActorWall ActorWall;

// Utility functions are in Structs/wall.h
struct Wall
{
	/// The first point of the wall
	Vector2 a;
	/// The second point of the wall
	Vector2 b;
	/// The fully qualified texture name (texture/level/uvtest.gtex instead of level/uvtest)
	char tex[80];
	/// The length of the wall (Call @c WallBake to update)
	float length;
	/// The angle of the wall (Call @c WallBake to update)
	float angle;
	/// The change in x over the length of the wall, calculated with @code Wall.b.x - Wall.a.x@endcode
	float dx;
	/// The change in y over the length of the wall, calculated with @code Wall.b.y - Wall.a.y@endcode
	float dy;
	/// The X scale of the texture
	float uvScale;
	/// The X offset of the texture
	float uvOffset;
	/// Jolt body ID
	JPH_BodyId bodyId;
};

struct ActorWall
{
	/// The first point of the wall
	Vector2 a;
	/// The second point of the wall
	Vector2 b;
	/// The fully qualified texture name (texture/level/uvtest.gtex instead of level/uvtest)
	char tex[80];
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
};

/**
 * Create a wall
 * @param a Wall start point
 * @param b Wall end point
 * @param texture Wall texture name
 * @param uvScale Wall texture scale
 * @param uvOffset Wall texture offset
 * @return Wall pointer
 */
Wall *CreateWall(Vector2 a, Vector2 b, const char *texture, float uvScale, float uvOffset);

/**
 * Create an infinitely thin collider for the wall's hitbox
 * @param wall The wall to create the hitbox for
 * @param bodyInterface The Jolt body interface used to look up the body from the body id
 * @note This function expects the wall to be baked
 */
void CreateWallCollider(Wall *wall, JPH_BodyInterface *bodyInterface);

/**
 * Frees the wall's physics body as well as the wall itself
 * @param bodyInterface The Jolt body interface used for accessing the wall's physics body
 * @param wall The wall struct
 */
void FreeWall(JPH_BodyInterface *bodyInterface, Wall *wall);

/**
 * Bake a wall's information
 * @param w Wall to bake
 */
void WallBake(Wall *w);

/**
 * Bake an actor wall's information
 * @param this ActorWall to bake
 */
void ActorWallBake(const Actor *this);

/**
 * Create a collider for an actor wall
 * @return The shape for the collider
 */
const JPH_Shape *ActorWallCreateCollider();

#endif //GAME_WALL_H
