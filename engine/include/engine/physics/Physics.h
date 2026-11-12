//
// Created by noah on 2/10/25.
//

/**
 * @file Physics.h
 * Physics constants and management
 */

#ifndef INIT_H
#define INIT_H

#include <engine/structs/GlobalState.h>
#include <engine/structs/Map.h>

/// Target physics updates per second (be careful with this)
#define PHYSICS_TARGET_TPS 60
/// Minimum physics updates per second. Delta time gets clamped to this valued.
#define PHYSICS_MIN_TPS 30

#define MAX_CONTACT_CONSTRAINTS 16384

#define PHYSICS_TARGET_MS (1000 / PHYSICS_TARGET_TPS)
#define PHYSICS_TARGET_MS_D (1000.0 / PHYSICS_TARGET_TPS)
#define PHYSICS_TARGET_NS (1000000000 / PHYSICS_TARGET_TPS) // nanoseconds because precision
#define PHYSICS_TARGET_NS_D (1000000000.0 / PHYSICS_TARGET_TPS)
#define PHYSICS_MIN_NS_D (1000000000.0 / PHYSICS_MIN_TPS)

/// Convert a Jolt @c Vector3 to a CGLM @c vec3
#define VECTOR3_TO_VEC3(vector) ((vec3){(vector).x, (vector).y, (vector).z})
/// Convert a CGLM @c vec3 to a Jolt @c Vector3
#define VEC3_TO_VECTOR3(vec) ((Vector3){(vec)[0], (vec)[1], (vec)[2]})

/// Convert a Jolt quaternion to a CGLM versor
#define QUAT_TO_VERSOR(src, dest) glm_quat_make(&(src).x, (dest))
/// Convert a CGLM versor to a Jolt quaternion
#define VERSOR_TO_QUAT(src, dest) \
	(dest).x = (src)[0]; \
	(dest).y = (src)[1]; \
	(dest).z = (src)[2]; \
	(dest).w = (src)[3];

/// Global gravity
#define GRAVITY (-156.96f) // -9.81 * 16

/// Types of physics object
enum ObjectLayers
{
	/// Static/unmoving
	OBJECT_LAYER_STATIC,
	/// Dynamic/moving
	OBJECT_LAYER_DYNAMIC,
	/// Player
	OBJECT_LAYER_PLAYER,
	/// Sensor/trigger
	OBJECT_LAYER_SENSOR,
};

enum BroadPhaseLayers
{
	BROAD_PHASE_LAYER_STATIC,
	BROAD_PHASE_LAYER_DYNAMIC,

	/// @warning Used for checking the number of broadphase layers, and as such is not a valid layer index
	BROADPHASE_LAYER_MAX,
};

/**
 * Initialize the physics-related global state that will persist across levels
 * @param state The GlobalState to initialize
 * @private
 */
void PhysicsInitGlobal(GlobalState *state);

/**
 * Cleanup the global physics state objects
 * @param state The GlobalState to clean up physics for
 * @private
 */
void PhysicsDestroyGlobal(const GlobalState *state);

/**
 * Initialize the physics related objects for a given map
 * @param map The map to initialize physics for
 * @private
 */
void PhysicsInitMap(Map *map);

/**
 * Cleanup the physics related objects associated with the given map
 * @param map The map to clean up the physics objects from
 * @private
 */
void PhysicsDestroyMap(const Map *map);

#endif //INIT_H
