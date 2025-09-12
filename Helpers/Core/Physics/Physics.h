//
// Created by noah on 2/10/25.
//

#ifndef INIT_H
#define INIT_H

#include <joltc/Physics/Body/BodyInterface.h>
#include "../../../config.h"
#include "../../../Structs/GlobalState.h"
#include "../../../Structs/Level.h"

#define PHYSICS_TARGET_MS (1000 / PHYSICS_TARGET_TPS)
#define PHYSICS_TARGET_MS_D (1000.0 / PHYSICS_TARGET_TPS)
#define PHYSICS_TARGET_NS (1000000000 / PHYSICS_TARGET_TPS) // nanoseconds because precision
#define PHYSICS_TARGET_NS_D (1000000000.0 / PHYSICS_TARGET_TPS)
#define PHYSICS_MIN_NS_D (1000000000.0 / PHYSICS_MIN_TPS)

#define VECTOR3_TO_VEC3(vector) ((vec3){(vector).x, (vector).y, (vector).z})
#define VEC3_TO_VECTOR3(vec) ((Vector3){(vec)[0], (vec)[1], (vec)[2]})

#define QUAT_TO_VERSOR(src, dest) glm_quat_make(&(src).x, (dest))
#define VERSOR_TO_QUAT(src, dest) \
	(dest).x = (src)[0]; \
	(dest).y = (src)[1]; \
	(dest).z = (src)[2]; \
	(dest).w = (src)[3];

enum ObjectLayers
{
	OBJECT_LAYER_STATIC,
	OBJECT_LAYER_DYNAMIC,
	OBJECT_LAYER_PLAYER,
	OBJECT_LAYER_SENSOR,
};

enum BroadPhaseLayers
{
	BROAD_PHASE_LAYER_STATIC,
	BROAD_PHASE_LAYER_DYNAMIC,

	/// @warning Used for checking the number of broadphase layers, and as such is not a valid layer index
	BROADPHASE_LAYER_MAX,
};

void PhysicsInitGlobal(GlobalState *state);

void PhysicsDestroyGlobal(const GlobalState *state);

void PhysicsInitLevel(Level *level);

void PhysicsDestroyLevel(const Level *level, JPH_BodyInterface *bodyInterface);

#endif //INIT_H
