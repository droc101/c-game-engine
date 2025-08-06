//
// Created by NBT22 on 8/5/25.
//

#include "RayCast.h"
#include "../../../Structs/GlobalState.h"

static bool BroadPhaseLayerShouldCollide(const JPH_BroadPhaseLayer layer)
{
	switch (layer)
	{
		case BROADPHASE_LAYER_STATIC:
		case BROADPHASE_LAYER_DYNAMIC:
			return true;
		default:
			return false;
	}
}

static bool ObjectLayerShouldCollide(const JPH_ObjectLayer layer)
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

static const JPH_BroadPhaseLayerFilter_Impl actorRayCastBroadPhaseLayerFilterImpl = {
	.ShouldCollide = BroadPhaseLayerShouldCollide,
};
static const JPH_ObjectLayerFilter_Impl actorRayCastObjectLayerFilterImpl = {
	.ShouldCollide = ObjectLayerShouldCollide,
};

static JPH_BroadPhaseLayerFilter *actorRayCastBroadPhaseLayerFilter;
static JPH_ObjectLayerFilter *actorRayCastObjectLayerFilter;
static JPH_BodyFilter *actorRayCastBodyFilter;
static JPH_ShapeFilter *actorRayCastShapeFilter;

static Actor *targetedEnemy = NULL;

static void RayCastCallback(void *context, const JPH_RayCastResult *result)
{
	const ActorRayCastOptions *options = context;
	targetedEnemy = (Actor *)JPH_BodyInterface_GetUserData(options->bodyInterface, result->bodyID);
	if (!targetedEnemy || (targetedEnemy->actorFlags & options->actorFlags) != options->actorFlags)
	{
		targetedEnemy = NULL;
	}
}

void RayCastInit()
{
	actorRayCastBroadPhaseLayerFilter = JPH_BroadPhaseLayerFilter_Create(&actorRayCastBroadPhaseLayerFilterImpl);
	actorRayCastObjectLayerFilter = JPH_ObjectLayerFilter_Create(&actorRayCastObjectLayerFilterImpl);
	actorRayCastBodyFilter = JPH_BodyFilter_Create(NULL);
	actorRayCastShapeFilter = JPH_ShapeFilter_Create(NULL);
}

void RayCastDestroy()
{
	JPH_BroadPhaseLayerFilter_Destroy(actorRayCastBroadPhaseLayerFilter);
	JPH_ObjectLayerFilter_Destroy(actorRayCastObjectLayerFilter);
	JPH_BodyFilter_Destroy(actorRayCastBodyFilter);
	JPH_ShapeFilter_Destroy(actorRayCastShapeFilter);
}

Actor *GetTargetedEnemy(const ActorRayCastOptions *options)
{
	const GlobalState *state = GetState();
	const JPH_NarrowPhaseQuery *narrowPhaseQuery = JPH_PhysicsSystem_GetNarrowPhaseQuery(state->level->physicsSystem);
	JPH_Quat rotation = {};
	JPH_Quat_FromEulerAngles(&state->level->player.transform.rotation, &rotation);
	Vector3 direction = {};
	JPH_Quat_Rotate(&rotation, &JPH_Vec3_Forward, &direction);
	Vector3 rayCastEnd = {};
	JPH_Vec3_MultiplyScalar(&direction, options->maxDistance, &rayCastEnd);
	const JPH_RayCastSettings rayCastSettings = {
		.treatConvexAsSolid = true,
	};
	if (!JPH_NarrowPhaseQuery_CastRay3(narrowPhaseQuery,
									   &state->level->player.transform.position,
									   &rayCastEnd,
									   &rayCastSettings,
									   JPH_CollisionCollectorType_ClosestHit,
									   &RayCastCallback,
									   (void *)options,
									   actorRayCastBroadPhaseLayerFilter,
									   actorRayCastObjectLayerFilter,
									   actorRayCastBodyFilter,
									   actorRayCastShapeFilter))
	{
		return NULL;
	}
	return targetedEnemy;
}

bool PerformRaycast(const Vector2 origin,
					const float angle,
					const float maxDistance,
					Vector2 *collisionPoint,
					const uint64_t category,
					const uint16_t mask)
{
	(void)origin;
	(void)angle;
	(void)maxDistance;
	(void)collisionPoint;
	(void)category;
	(void)mask;
	// const GlobalState *state = GetState();
	// Vector2 rayEnd = Vector2FromAngle(angle);
	// rayEnd = Vector2Scale(rayEnd, maxDistance);
	// Vector2 raycastHit = v2s(FP_NAN);
	// b2World_CastRay(state->level->worldId,
	// 				origin,
	// 				rayEnd,
	// 				(b2QueryFilter){.categoryBits = category, .maskBits = mask},
	// 				RaycastCallback_GetPosition,
	// 				&raycastHit);
	//
	// if (raycastHit.x != FP_NAN)
	// {
	// 	*collisionPoint = raycastHit;
	// 	return true;
	// }
	return false;
}
