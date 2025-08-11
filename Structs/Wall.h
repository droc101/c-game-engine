//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_WALL_H
#define GAME_WALL_H

#include "../defines.h"

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

#endif //GAME_WALL_H
