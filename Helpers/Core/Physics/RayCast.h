//
// Created by NBT22 on 8/5/25.
//

#ifndef RAYCAST_H
#define RAYCAST_H

#include "../../../defines.h"

/**
 * Get the enemy that the player is targeting
 * @param maxDistance The maximum distance to check for enemies
 * @return The actor struct, or NULL if no enemy is targeted
 */
Actor *GetTargetedEnemy(float maxDistance);

bool PerformRaycast(Vector2 origin,
					float angle,
					float maxDistance,
					Vector2 *collisionPoint,
					uint64_t category,
					uint16_t mask);

#endif //RAYCAST_H
