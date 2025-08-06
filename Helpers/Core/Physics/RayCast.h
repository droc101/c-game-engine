//
// Created by NBT22 on 8/5/25.
//

#ifndef RAYCAST_H
#define RAYCAST_H

#include "../../../defines.h"

typedef struct ActorRayCastOptions ActorRayCastOptions;

struct ActorRayCastOptions
{
	JPH_BodyInterface *bodyInterface;
	float maxDistance;
	uint32_t actorFlags;
};

void RayCastInit();

void RayCastDestroy();

/**
 * Get the enemy that the player is targeting
 * @param options The options with which to perform the ray cast
 * @return The actor struct, or NULL if no enemy is targeted
 */
Actor *GetTargetedEnemy(const ActorRayCastOptions *options);

bool PerformRaycast(Vector2 origin,
					float angle,
					float maxDistance,
					Vector2 *collisionPoint,
					uint64_t category,
					uint16_t mask);

#endif //RAYCAST_H
