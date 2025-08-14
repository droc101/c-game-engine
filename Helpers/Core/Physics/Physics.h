//
// Created by noah on 2/10/25.
//

#ifndef INIT_H
#define INIT_H

#include <joltc.h>
#include "../../../Structs/GlobalState.h"
#include "../../../Structs/Level.h"

#define PHYSICS_TARGET_MS (1000 / PHYSICS_TARGET_TPS)
#define PHYSICS_TARGET_MS_D (1000.0 / PHYSICS_TARGET_TPS)
#define PHYSICS_TARGET_NS (1000000000 / PHYSICS_TARGET_TPS) // nanoseconds because precision
#define PHYSICS_TARGET_NS_D (1000000000.0 / PHYSICS_TARGET_TPS)

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
