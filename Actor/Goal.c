//
// Created by droc101 on 7/12/2024.
//

#include "Goal.h"
#include <math.h>
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Structs/Actor.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"

#define GOAL_INPUT_ENABLE 1
#define GOAL_INPUT_DISABLE 2

#define GOAL_OUTPUT_COLLECTED 2

typedef struct GoalData
{
	bool enabled;
} GoalData;

bool GoalSignalHandler(Actor *this, const Actor *sender, const byte signal, const Param *param)
{
	if (DefaultSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	GoalData *data = this->extraData;
	if (signal == GOAL_INPUT_ENABLE && !data->enabled)
	{
		data->enabled = true;
		strcpy(this->actorWall->tex, TEXTURE("actor_goal0"));
		return true;
	}
	if (signal == GOAL_INPUT_DISABLE && data->enabled)
	{
		data->enabled = false;
		strcpy(this->actorWall->tex, TEXTURE("actor_goal1"));
		return true;
	}
	return false;
}

void CreateGoalSensor(Actor *this, const Transform *transform)
{
	const JPH_Shape *shape = (const JPH_Shape *)JPH_BoxShape_Create((Vector3[]){{0.5f, 0.5f, 0.5f}},
																	JPH_DEFAULT_CONVEX_RADIUS);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create3(shape,
																					  &transform->position,
																					  &JPH_Quat_Identity,
																					  JPH_MotionType_Static,
																					  OBJECT_LAYER_SENSOR);
	JPH_BodyCreationSettings_SetUserData(bodyCreationSettings, (uint64_t)this);
	JPH_BodyCreationSettings_SetIsSensor(bodyCreationSettings, true);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}

void GoalOnPlayerContactAdded(Actor *this, JPH_BodyId /*bodyId*/)
{
	const GoalData *data = this->extraData;
	if (data->enabled)
	{
		GetState()->saveData->coins += 10;
		ActorFireOutput(this, GOAL_OUTPUT_COLLECTED, PARAM_NONE);
		RemoveActor(this);
	}
}

void GoalInit(Actor *this, const KvList *params, Transform *transform)
{
	this->SignalHandler = GoalSignalHandler;
	GoalData *data = calloc(1, sizeof(GoalData));
	CheckAlloc(data);
	this->extraData = data;
	data->enabled = KvGetBool(params, "startEnabled", true);

	this->actorWall = malloc(sizeof(ActorWall));
	this->actorWall->a = v2(0, 0.5f);
	this->actorWall->b = v2(0, -0.5f);
	strncpy(this->actorWall->tex, data->enabled ? TEXTURE("actor_goal0") : TEXTURE("actor_goal1"), 80);
	this->actorWall->uvScale = 1.0f;
	this->actorWall->uvOffset = 0.0f;
	this->actorWall->height = 1.0f;
	ActorWallBake(this);

	this->OnPlayerContactAdded = GoalOnPlayerContactAdded;

	CreateGoalSensor(this, transform);
}

void GoalUpdate(Actor *this, double /*delta*/)
{
	Vector3 position = {};
	JPH_BodyInterface_GetPosition(this->bodyInterface, this->bodyId, &position);
	const float rotation = atan2f(GetState()->level->player.transform.position.z - position.z,
								  GetState()->level->player.transform.position.x - position.x) +
						   GLM_PI_2f;
	this->actorWall->a = v2(0.5f * cosf(rotation), 0.5f * sinf(rotation));
	this->actorWall->b = v2(-0.5f * cosf(rotation), -0.5f * sinf(rotation));
}
