//
// Created by droc101 on 4/28/25.
//

#include "actor/Laser.h"
#include <engine/assets/AssetReader.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/ActorWall.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Param.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/Body.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <joltc/types.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct LaserData
{
	LaserHeight height;
	bool on;
} LaserData;

static bool ActorRaycastBroadPhaseLayerShouldCollide(const JPH_BroadPhaseLayer layer)
{
	switch (layer)
	{
		case BROAD_PHASE_LAYER_STATIC:
		case BROAD_PHASE_LAYER_DYNAMIC:
			return true;
		default:
			return false;
	}
}

static bool ActorRaycastObjectLayerShouldCollide(const JPH_ObjectLayer layer)
{
	switch (layer)
	{
		case OBJECT_LAYER_STATIC:
		case OBJECT_LAYER_DYNAMIC:
			return true;
		default:
			return false;
	}
}

static bool TripleLaserBroadPhaseLayerShouldCollide(const JPH_BroadPhaseLayer layer)
{
	return layer == BROAD_PHASE_LAYER_STATIC;
}

static bool TripleLaserObjectLayerShouldCollide(const JPH_ObjectLayer layer)
{
	return layer == OBJECT_LAYER_STATIC;
}

static bool BodyFilterShouldCollide(const JPH_BodyId bodyId)
{
	JPH_BodyInterface *bodyInterface = JPH_PhysicsSystem_GetBodyInterface(GetState()->map->physicsSystem);
	const Actor *actor = (const Actor *)JPH_BodyInterface_GetUserData(bodyInterface, bodyId);
	return !actor || ((actor->actorFlags & ACTOR_FLAG_CAN_BLOCK_LASERS) == ACTOR_FLAG_CAN_BLOCK_LASERS);
}

static bool BodyFilterShouldCollideLocked(const JPH_Body *body)
{
	const Actor *actor = (const Actor *)JPH_Body_GetUserData(body);
	return !actor || ((actor->actorFlags & ACTOR_FLAG_CAN_BLOCK_LASERS) == ACTOR_FLAG_CAN_BLOCK_LASERS);
}

static const JPH_BroadPhaseLayerFilter_Impl normalLaserBroadPhaseLayerFilterImpl = {
	.ShouldCollide = ActorRaycastBroadPhaseLayerShouldCollide,
};
static const JPH_ObjectLayerFilter_Impl normalLaserObjectLayerFilterImpl = {
	.ShouldCollide = ActorRaycastObjectLayerShouldCollide,
};
static const JPH_BroadPhaseLayerFilter_Impl tripleLaserBroadPhaseLayerFilterImpl = {
	.ShouldCollide = TripleLaserBroadPhaseLayerShouldCollide,
};
static const JPH_ObjectLayerFilter_Impl tripleLaserObjectLayerFilterImpl = {
	.ShouldCollide = TripleLaserObjectLayerShouldCollide,
};
static const JPH_BodyFilter_Impl bodyFilterImpl = {
	.ShouldCollide = BodyFilterShouldCollide,
	.ShouldCollideLocked = BodyFilterShouldCollideLocked,
};

static JPH_BroadPhaseLayerFilter *normalLaserBroadPhaseLayerFilter;
static JPH_ObjectLayerFilter *normalLaserObjectLayerFilter;
static JPH_BroadPhaseLayerFilter *tripleLaserBroadPhaseLayerFilter;
static JPH_ObjectLayerFilter *tripleLaserObjectLayerFilter;
static JPH_BodyFilter *bodyFilter;

static const float maxDistance = 50.0f;

static inline void LaserCreateBody(Actor *this, const Transform *transform)
{
	JPH_ShapeSettings *shapeSettings = (JPH_ShapeSettings *)JPH_EmptyShapeSettings_Create(&Vector3_Zero);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create_GAME(shapeSettings,
																						  transform,
																						  JPH_MotionType_Static,
																						  OBJECT_LAYER_STATIC,
																						  this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_DontActivate);
	JPH_ShapeSettings_Destroy(shapeSettings);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
static void LaserUpdate(Actor *this, double delta)
{
	const LaserData *data = this->extraData;
	if (data->on)
	{
		const JPH_PhysicsSystem *physicsSystem = GetState()->map->physicsSystem;
		const JPH_NarrowPhaseQuery *narrowPhaseQuery = JPH_PhysicsSystem_GetNarrowPhaseQuery(physicsSystem);
		JPH_RayCastResult result = {};
		Vector3 hitPointOffset = {};
		JPH_BroadPhaseLayerFilter *broadPhaseLayerFilter = data->height == LASER_HEIGHT_TRIPLE
																   ? tripleLaserBroadPhaseLayerFilter
																   : normalLaserBroadPhaseLayerFilter;
		JPH_ObjectLayerFilter *objectLayerFilter = data->height == LASER_HEIGHT_TRIPLE ? tripleLaserObjectLayerFilter
																					   : normalLaserObjectLayerFilter;
		const bool hit = JPH_NarrowPhaseQuery_CastRay2_GAME(narrowPhaseQuery,
															this->bodyInterface,
															this->bodyId,
															maxDistance,
															&result,
															&hitPointOffset,
															broadPhaseLayerFilter,
															objectLayerFilter,
															bodyFilter);
		if (hit)
		{
			this->actorWall->b = v2(hitPointOffset.x, hitPointOffset.z);
			ActorWallBake(this);
		}
		this->actorWall->uvOffset = (float)fmod(this->actorWall->uvOffset + delta / 8, 1.0);
	}
}

static void LaserTurnOnHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	LaserData *data = this->extraData;
	data->on = true;
}

static void LaserTurnOffHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	LaserData *data = this->extraData;
	data->on = false;
	this->actorWall->b = v2(0.01, 0);
	ActorWallBake(this);
}

void LaserInit(Actor *this, const KvList params, Transform *transform)
{
	LaserData *data = calloc(1, sizeof(LaserData));
	CheckAlloc(data);
	this->extraData = data;
	data->height = KvGetByte(params, "height", LASER_HEIGHT_MIDDLE);
	data->on = KvGetBool(params, "startOn", true);

	this->actorWall = malloc(sizeof(ActorWall));
	CheckAlloc(this->actorWall);
	this->actorWall->a = v2s(0);
	this->actorWall->b = v2s(0);
	this->actorWall->tex = malloc(strlen(TEXTURE("actor/triplelaser") + 1));
	strcpy(this->actorWall->tex,
		   data->height == LASER_HEIGHT_TRIPLE ? TEXTURE("actor/triplelaser") : TEXTURE("actor/laser"));
	this->actorWall->uvScale = 1.0f;
	this->actorWall->uvOffset = 0.0f;
	this->actorWall->height = 1.0f;

	if (!data->on)
	{
		this->actorWall->b = v2(0.01, 0);
		ActorWallBake(this);
	}

	switch (data->height)
	{
		case LASER_HEIGHT_FLOOR:
			transform->position.y += -0.3f;
			break;
		case LASER_HEIGHT_CEILING:
			transform->position.y += 0.3f;
			break;
		case LASER_HEIGHT_MIDDLE:
		default:
			transform->position.y += 0.0f;
			break;
	}
	ActorWallBake(this);

	LaserCreateBody(this, transform);

	// TODO: Make harmful - Depends on being able to take damage
}

void LaserRaycastFiltersInit()
{
	normalLaserBroadPhaseLayerFilter = JPH_BroadPhaseLayerFilter_Create(&normalLaserBroadPhaseLayerFilterImpl);
	normalLaserObjectLayerFilter = JPH_ObjectLayerFilter_Create(&normalLaserObjectLayerFilterImpl);
	tripleLaserBroadPhaseLayerFilter = JPH_BroadPhaseLayerFilter_Create(&tripleLaserBroadPhaseLayerFilterImpl);
	tripleLaserObjectLayerFilter = JPH_ObjectLayerFilter_Create(&tripleLaserObjectLayerFilterImpl);
	bodyFilter = JPH_BodyFilter_Create(&bodyFilterImpl);
}

void LaserRaycastFiltersDestroy()
{
	JPH_BroadPhaseLayerFilter_Destroy(normalLaserBroadPhaseLayerFilter);
	JPH_ObjectLayerFilter_Destroy(normalLaserObjectLayerFilter);
	JPH_BroadPhaseLayerFilter_Destroy(tripleLaserBroadPhaseLayerFilter);
	JPH_ObjectLayerFilter_Destroy(tripleLaserObjectLayerFilter);
	JPH_BodyFilter_Destroy(bodyFilter);
}

static ActorDefinition definition = {.actorType = ACTOR_TYPE_LASER,
									 .Update = LaserUpdate,
									 .OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
									 .OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
									 .OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
									 .RenderUi = DefaultActorRenderUi,
									 .Destroy = DefaultActorDestroy,
									 .Init = LaserInit};

void RegisterLaser()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActorInput(&definition, LASER_INPUT_TURN_ON, LaserTurnOnHandler);
	RegisterActorInput(&definition, LASER_INPUT_TURN_OFF, LaserTurnOffHandler);
	RegisterActor(LASER_ACTOR_NAME, &definition);
}
