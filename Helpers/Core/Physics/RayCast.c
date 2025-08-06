//
// Created by NBT22 on 8/5/25.
//

#include "RayCast.h"
#include "../../../Structs/GlobalState.h"

static void RayCastCallback(void *context, const JPH_RayCastResult *result)
{

}

Actor *GetTargetedEnemy(const float maxDistance)
{
	const GlobalState *state = GetState();
	// Vector2 rayEnd = Vector2FromAngle(state->level->player.transform.rotation.y);
	// rayEnd = Vector2Scale(rayEnd, maxDistance);
	// b2ShapeId raycastHit = b2_nullShapeId;
	// b2World_CastRay(state->level->worldId,
	// 				state->level->player.position,
	// 				rayEnd,
	// 				(b2QueryFilter){.categoryBits = COLLISION_GROUP_PLAYER, .maskBits = COLLISION_GROUP_HURTBOX},
	// 				RaycastCallback,
	// 				&raycastHit);
	//
	// if (b2Shape_IsValid(raycastHit))
	// {
	// 	ListLock(state->level->actors);
	// 	for (int i = 0; i < state->level->actors.length; i++)
	// 	{
	// 		Actor *actor = ListGetPointer(state->level->actors, i);
	// 		if (b2Shape_GetBody(raycastHit).index1 == actor->bodyId.index1)
	// 		{
	// 			ListUnlock(state->level->actors);
	// 			return actor;
	// 		}
	// 	}
	// 	ListUnlock(state->level->actors);
	// }

	/*
	JPH_CAPI bool JPH_NarrowPhaseQuery_CastRay3(const JPH_NarrowPhaseQuery *query,
												const JPH_RVec3 *origin,
												const JPH_Vec3 *direction,
												const JPH_RayCastSettings *rayCastSettings,
												JPH_CollisionCollectorType collectorType,
												JPH_CastRayResultCallback *callback,
												void *userData,
												JPH_BroadPhaseLayerFilter *broadPhaseLayerFilter,
												JPH_ObjectLayerFilter *objectLayerFilter,
												const JPH_BodyFilter *bodyFilter,
												const JPH_ShapeFilter *shapeFilter);
	*/
	// const JPH_NarrowPhaseQuery *narrowPhaseQuery = JPH_PhysicsSystem_GetNarrowPhaseQuery(state->level->physicsSystem);
	// JPH_Quat rotation = {};
	// JPH_CharacterVirtual_GetRotation(state->level->player.joltCharacter, &rotation);
	// Vector3 direction = {};
	// JPH_Quat_Rotate(&rotation, &JPH_Vec3_Forward, &direction);
	// JPH_RayCastSettings rayCastSettings = {
	// 	.treatConvexAsSolid = true,
	// };
	//
	// JPH_NarrowPhaseQuery_CastRay3(narrowPhaseQuery, &state->level->player.transform.position, &direction, &rayCastSettings, JPH_CollisionCollectorType_ClosestHit, &RayCastCallback, NULL, );
	return NULL;
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
