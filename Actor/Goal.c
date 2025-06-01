//
// Created by droc101 on 7/12/2024.
//

#include "Goal.h"
#include <box2d/box2d.h>
#include <math.h>

#include "../Helpers/Collision.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/TextBox.h"
#include "../Structs/Actor.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

#define GOAL_OUTPUT_COLLECTED 2

#define GOAL_INPUT_ENABLE 1
#define GOAL_INPUT_DISABLE 2

typedef struct GoalData
{
	b2ShapeId shapeId;
	bool enabled;
} GoalData;

bool GoalSignalHandler(Actor *self, const Actor *sender, byte signal, const Param *param)
{
	if (DefaultSignalHandler(self, sender, signal, param))
	{
		return true;
	}
	GoalData *data = self->extraData;
	if (signal == GOAL_INPUT_ENABLE && !data->enabled)
	{
		data->enabled = true;
		strcpy(self->actorWall->tex, TEXTURE("actor_goal0"));
		return true;
	}
	if (signal == GOAL_INPUT_DISABLE && data->enabled)
	{
		data->enabled = false;
		strcpy(self->actorWall->tex, TEXTURE("actor_goal1"));
		return true;
	}
	return false;
}

void CreateGoalSensor(Actor *this, const b2WorldId worldId)
{
	GoalData *goalData = (GoalData *)this->extraData;

	b2BodyDef sensorBodyDef = b2DefaultBodyDef();
	sensorBodyDef.type = b2_staticBody;
	sensorBodyDef.position = this->position;
	this->bodyId = b2CreateBody(worldId, &sensorBodyDef);
	this->actorWall->bodyId = this->bodyId;
	const b2Circle sensorShape = {
		.radius = 0.5f,
	};
	b2ShapeDef sensorShapeDef = b2DefaultShapeDef();
	sensorShapeDef.isSensor = true;
	sensorShapeDef.filter.categoryBits = COLLISION_GROUP_ACTOR;
	sensorShapeDef.filter.maskBits = COLLISION_GROUP_PLAYER;
	goalData->shapeId = b2CreateCircleShape(this->bodyId, &sensorShapeDef, &sensorShape);
}

void GoalInit(Actor *this, const b2WorldId worldId, const KvList *params)
{
	this->SignalHandler = GoalSignalHandler;
	this->extraData = calloc(1, sizeof(GoalData));
	CheckAlloc(this->extraData);
	GoalData *goalData = (GoalData *)this->extraData;
	goalData->enabled = KvGetBool(params, "startEnabled", true);

	this->actorWall = CreateWall(v2(0, 0.5f),
								 v2(0, -0.5f),
								 goalData->enabled ? TEXTURE("actor_goal0") : TEXTURE("actor_goal1"),
								 1.0f,
								 0.0f);
	WallBake(this->actorWall);

	CreateGoalSensor(this, worldId);
}

void GoalUpdate(Actor *this, double /*delta*/)
{
	const GoalData *goalData = (GoalData *)this->extraData;

	const Vector2 playerPosition = GetState()->level->player.pos;
	const float rotation = atan2f(playerPosition.y - this->position.y, playerPosition.x - this->position.x) + PIf / 2;
	this->actorWall->a = v2(0.5f * cosf(rotation), 0.5f * sinf(rotation));
	this->actorWall->b = v2(-0.5f * cosf(rotation), -0.5f * sinf(rotation));

	if (goalData->enabled)
	{
		if (GetSensorState(GetState()->level->worldId, goalData->shapeId.index1, false))
		{
			const TextBox tb = DEFINE_TEXT("Goal!",
										   2,
										   20,
										   0,
										   70,
										   TEXT_BOX_H_ALIGN_CENTER,
										   TEXT_BOX_V_ALIGN_TOP,
										   TEXT_BOX_THEME_WHITE);
			ShowTextBox(tb);
			ActorFireOutput(this, GOAL_OUTPUT_COLLECTED, PARAM_NONE);
			RemoveActor(this);
		}
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GoalDestroy(Actor *this)
{
	b2DestroyBody(this->bodyId);
	free(this->actorWall);
	free(this->extraData);
}
