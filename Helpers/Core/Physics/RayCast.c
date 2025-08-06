//
// Created by NBT22 on 8/5/25.
//

#include "RayCast.h"
#include "../../../Structs/GlobalState.h"

static bool NormalLaserBroadPhaseLayerShouldCollide(const JPH_BroadPhaseLayer layer)
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

static bool NormalLaserObjectLayerShouldCollide(const JPH_ObjectLayer layer)
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
	.ShouldCollide = NormalLaserBroadPhaseLayerShouldCollide,
};
static const JPH_ObjectLayerFilter_Impl actorRayCastObjectLayerFilterImpl = {
	.ShouldCollide = NormalLaserObjectLayerShouldCollide,
};

static JPH_BroadPhaseLayerFilter *actorRayCastBroadPhaseLayerFilter;
static JPH_ObjectLayerFilter *actorRayCastObjectLayerFilter;

void RayCastInit()
{
	actorRayCastBroadPhaseLayerFilter = JPH_BroadPhaseLayerFilter_Create(&actorRayCastBroadPhaseLayerFilterImpl);
	actorRayCastObjectLayerFilter = JPH_ObjectLayerFilter_Create(&actorRayCastObjectLayerFilterImpl);
}

void RayCastDestroy()
{
	JPH_BroadPhaseLayerFilter_Destroy(actorRayCastBroadPhaseLayerFilter);
	JPH_ObjectLayerFilter_Destroy(actorRayCastObjectLayerFilter);
}

Actor *GetTargetedEnemy(const ActorRayCastOptions *options)
{
	const GlobalState *state = GetState();
	const JPH_NarrowPhaseQuery *narrowPhaseQuery = JPH_PhysicsSystem_GetNarrowPhaseQuery(state->level->physicsSystem);
	JPH_RayCastResult rayCastResult = {};
	if (!JPH_NarrowPhaseQuery_CastRay_GAME(narrowPhaseQuery,
										   &state->level->player.transform,
										   options->maxDistance,
										   &rayCastResult,
										   actorRayCastBroadPhaseLayerFilter,
										   actorRayCastObjectLayerFilter))
	{
		return NULL;
	}
	Actor *targetedEnemy = (Actor *)JPH_BodyInterface_GetUserData(options->bodyInterface, rayCastResult.bodyID);
	if (!targetedEnemy || (targetedEnemy->actorFlags & options->actorFlags) != options->actorFlags)
	{
		return NULL;
	}
	return targetedEnemy;
}
