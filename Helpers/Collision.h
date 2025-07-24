//
// Created by noah on 2/10/25.
//

#ifndef COLLISION_H
#define COLLISION_H

#include "../defines.h"

/**
 * Box2D raycast callback function
 * @param shapeId The shape hit by the ray
 * @param point Unused
 * @param normal Unused
 * @param fraction The fraction along the ray at the point of intersection
 * @param raycastHit A pointer to a shapeId that is used to store the closest hit shape
 * @return The @c fraction argument
 *
 * @see https://box2d.org/documentation/group__world.html#gad34f863cfebe93a7d6448c30e30f6a01
 */
float RaycastCallback(int shapeId, Vector2 point, Vector2 normal, float fraction, void *raycastHit);

/**
 * Get the state of a sensor. The current state of the sensor must be passed into this function because Box2D sensors
 * only provide events when collision starts or stops
 * @param worldId The Box2D world that the sensor is in
 * @param sensorShapeIdIndex The @c index1 member of the sensor shape ID
 * @param currentState A boolean for the current state of the sensor
 * @return The new state of the sensor. True if something is colliding with it, false if nothing is colliding with it
 */
bool GetSensorState(int worldId, uint sensorShapeIdIndex, bool currentState);

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

#endif //COLLISION_H
