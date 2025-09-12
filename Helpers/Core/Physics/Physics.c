//
// Created by noah on 2/10/25.
//

#include "Physics.h"
#include <joltc/constants.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <joltc/types.h>
#include <stdbool.h>
#include <stddef.h>
#include "../../../Actor/Laser.h"
#include "../../../Debug/JoltDebugRenderer.h"
#include "../../../Structs/GlobalState.h"
#include "../Error.h"
#include "Player.h"

static JPH_BroadPhaseLayer GetBroadPhaseLayer(const JPH_ObjectLayer inLayer)
{
	switch (inLayer)
	{
		case OBJECT_LAYER_STATIC:
		case OBJECT_LAYER_SENSOR:
			return BROAD_PHASE_LAYER_STATIC;
		case OBJECT_LAYER_DYNAMIC:
		case OBJECT_LAYER_PLAYER:
			return BROAD_PHASE_LAYER_DYNAMIC;
		default:
			return JPH_BroadPhaseLayerInvalid;
	}
}

static bool ObjectLayerShouldCollide(const JPH_ObjectLayer inLayer1, const JPH_ObjectLayer inLayer2)
{
	if (inLayer1 == OBJECT_LAYER_DYNAMIC || inLayer1 == OBJECT_LAYER_PLAYER)
	{
		switch (inLayer2)
		{
			case OBJECT_LAYER_STATIC:
			case OBJECT_LAYER_DYNAMIC:
			case OBJECT_LAYER_SENSOR:
				return true;
			default:
				return false;
		}
	}
	return false;
}

static bool ObjectVsBroadPhaseLayerShouldCollide(const JPH_ObjectLayer inObjectLayer,
												 const JPH_BroadPhaseLayer /*inBroadPhaseLayer*/)
{
	if (GetBroadPhaseLayer(inObjectLayer) == BROAD_PHASE_LAYER_STATIC)
	{
		return false;
	}
	return true;
}

static const JPH_ObjectVsBroadPhaseLayerFilter_Impl objectVsBroadPhaseLayerFilterImpl = {
	.ShouldCollide = ObjectVsBroadPhaseLayerShouldCollide,
};
static const JPH_ObjectLayerPairFilter_Impl objectLayerPairFilterImpl = {
	.ShouldCollide = ObjectLayerShouldCollide,
};
static const JPH_BroadPhaseLayerInterface_Impl broadPhaseLayerInterfaceImpl = {
	.GetBroadPhaseLayer = GetBroadPhaseLayer,
};

void PhysicsInitGlobal(GlobalState *state)
{
	if (!JPH_Init())
	{
		Error("Failed to initialize Jolt Physics!");
	}
	state->jobSystem = JPH_JobSystemThreadPool_Create(NULL);
	JoltDebugRendererInit();
	PlayerPersistentStateInit();
	LaserRaycastFiltersInit();
}

void PhysicsDestroyGlobal(const GlobalState *state)
{
	JoltDebugRendererDestroy();
	PlayerPersistentStateDestroy();
	LaserRaycastFiltersDestroy();
	JPH_JobSystem_Destroy(state->jobSystem);
	JPH_Shutdown();
}

void PhysicsInitLevel(Level *level)
{
	const JPH_PhysicsSystemSettings physicsSystemSettings = {
		.maxContactConstraints = 16384, // TODO magic number BAD
		.broadPhaseLayerInterface = JPH_BroadPhaseLayerInterface_Create(BROADPHASE_LAYER_MAX,
																		&broadPhaseLayerInterfaceImpl),
		.objectLayerPairFilter = JPH_ObjectLayerPairFilter_Create(&objectLayerPairFilterImpl),
		.objectVsBroadPhaseLayerFilter = JPH_ObjectVsBroadPhaseLayerFilter_Create(&objectVsBroadPhaseLayerFilterImpl),
	};
	level->physicsSystem = JPH_PhysicsSystem_Create(&physicsSystemSettings);

	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(level->physicsSystem);
	const JPH_Plane plane = {
		.normal.y = 1,
	};
	JPH_Shape *shape = (JPH_Shape *)JPH_PlaneShape_Create(&plane, NULL, 100);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create3(shape,
																					  (Vector3[]){{0.0f, -0.5f, 0.0f}},
																					  &JPH_Quat_Identity,
																					  JPH_MotionType_Static,
																					  OBJECT_LAYER_STATIC);
	JPH_BodyCreationSettings_SetFriction(bodyCreationSettings, 4.25f);
	level->floorBodyId = JPH_BodyInterface_CreateAndAddBody(bodyInterface,
															bodyCreationSettings,
															JPH_Activation_DontActivate);
	JPH_Shape_Destroy(shape);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

void PhysicsDestroyLevel(const Level *level, JPH_BodyInterface *bodyInterface)
{
	JPH_CharacterVirtual_Destroy(level->player.joltCharacter);
	JPH_BodyInterface_RemoveAndDestroyBody(bodyInterface, level->floorBodyId);
	JPH_PhysicsSystem_Destroy(level->physicsSystem);
}
