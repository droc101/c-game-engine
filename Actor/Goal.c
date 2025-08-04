//
// Created by droc101 on 7/12/2024.
//

#include "Goal.h"
#include <math.h>
#include "../Helpers/Collision.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/MathEx.h"
#include "../Structs/Actor.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

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

void CreateGoalSensor(Actor *this)
{
	const JPH_Shape *shape = (const JPH_Shape *)JPH_SphereShape_Create(0.5f);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create3(shape,
																					  &this->transform.position,
																					  NULL,
																					  JPH_MotionType_Static,
																					  OBJECT_LAYER_SENSOR);
	JPH_BodyCreationSettings_SetUserData(bodyCreationSettings, (uint64_t)this);
	JPH_BodyCreationSettings_SetIsSensor(bodyCreationSettings, true);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_Activate);
}

void GoalOnPlayerContactAdded(Actor *this)
{
	const GoalData *data = this->extraData;
	if (data->enabled)
	{
		GetState()->saveData->coins += 10;
		ActorFireOutput(this, GOAL_OUTPUT_COLLECTED, PARAM_NONE);
		RemoveActor(this);
	}
}

void GoalInit(Actor *this, const KvList *params)
{
	this->SignalHandler = GoalSignalHandler;
	GoalData *data = calloc(1, sizeof(GoalData));
	CheckAlloc(data);
	this->extraData = data;
	data->enabled = KvGetBool(params, "startEnabled", true);

	this->actorWall = CreateWall(v2(0, 0.5f),
								 v2(0, -0.5f),
								 data->enabled ? TEXTURE("actor_goal0") : TEXTURE("actor_goal1"),
								 1.0f,
								 0.0f);
	WallBake(this->actorWall);

	this->OnPlayerContactAdded = GoalOnPlayerContactAdded;

	CreateGoalSensor(this);
}

void GoalUpdate(Actor *this, double /*delta*/)
{
	const float rotation = atan2f(GetState()->level->player.transform.position.z - this->transform.position.z,
								  GetState()->level->player.transform.position.x - this->transform.position.x) +
						   PIf / 2;
	this->actorWall->a = v2(0.5f * cosf(rotation), 0.5f * sinf(rotation));
	this->actorWall->b = v2(-0.5f * cosf(rotation), -0.5f * sinf(rotation));
}
