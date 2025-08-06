//
// Created by NBT22 on 8/5/25.
//

#include "RayCast.h"
#include "../../../Structs/GlobalState.h"

static bool BroadPhaseLayerShouldCollide(void * /*userData*/, const JPH_BroadPhaseLayer layer)
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

static bool ObjectLayerShouldCollide(void * /*userData*/, const JPH_ObjectLayer layer)
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

static bool BodyFilterShouldCollide(void *userData, const JPH_BodyId bodyId)
{
	const ActorRayCastOptions *rayCastOptions = userData;
	const Actor *actor = (const Actor *)JPH_BodyInterface_GetUserData(rayCastOptions->bodyInterface, bodyId);
	return actor && ((actor->actorFlags & rayCastOptions->actorFlags) == rayCastOptions->actorFlags);
}

static bool BodyFilterShouldCollideLocked(void *userData, const JPH_Body *body)
{
	const ActorRayCastOptions *rayCastOptions = userData;
	const Actor *actor = (const Actor *)JPH_Body_GetUserData(body);
	return (actor->actorFlags & rayCastOptions->actorFlags) == rayCastOptions->actorFlags;
}

static bool ShapeFilterShouldCollide(void * /*userData*/,
									 const JPH_Shape * /*inShape2*/,
									 JPH_SubShapeId /*inSubShapeIDOfShape2*/)
{
	return true;
}

static const JPH_BroadPhaseLayerFilter_Impl actorRayCastBroadPhaseLayerFilterImpl = {
	.ShouldCollide = BroadPhaseLayerShouldCollide,
};
static const JPH_ObjectLayerFilter_Impl actorRayCastObjectLayerFilterImpl = {
	.ShouldCollide = ObjectLayerShouldCollide,
};
static const JPH_BodyFilter_Impl actorRayCastBodyFilterImpl = {
	.ShouldCollide = BodyFilterShouldCollide,
	.ShouldCollideLocked = BodyFilterShouldCollideLocked,
};
static const JPH_ShapeFilter_Impl actorRayCastShapeFilterImpl = {
	.ShouldCollide = ShapeFilterShouldCollide,
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
}

void RayCastInit()
{
	actorRayCastBroadPhaseLayerFilter = JPH_BroadPhaseLayerFilter_Create(NULL, &actorRayCastBroadPhaseLayerFilterImpl);
	actorRayCastObjectLayerFilter = JPH_ObjectLayerFilter_Create(NULL, &actorRayCastObjectLayerFilterImpl);
	actorRayCastBodyFilter = JPH_BodyFilter_Create(NULL, &actorRayCastBodyFilterImpl);
	actorRayCastShapeFilter = JPH_ShapeFilter_Create(NULL, &actorRayCastShapeFilterImpl);
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
	JPH_BroadPhaseLayerFilter_SetUserData(actorRayCastBroadPhaseLayerFilter, (void *)options);
	JPH_ObjectLayerFilter_SetUserData(actorRayCastObjectLayerFilter, (void *)options);
	JPH_BodyFilter_SetUserData(actorRayCastBodyFilter, (void *)options);
	JPH_ShapeFilter_SetUserData(actorRayCastShapeFilter, (void *)options);

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
	const bool hitEnemy = JPH_NarrowPhaseQuery_CastRay3(narrowPhaseQuery,
														&state->level->player.transform.position,
														&rayCastEnd,
														&rayCastSettings,
														JPH_CollisionCollectorType_ClosestHit,
														&RayCastCallback,
														(void *)options,
														actorRayCastBroadPhaseLayerFilter,
														actorRayCastObjectLayerFilter,
														actorRayCastBodyFilter,
														actorRayCastShapeFilter);
	if (!hitEnemy)
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
