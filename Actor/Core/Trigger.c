//
// Created by droc101 on 4/13/25.
//

#include "Trigger.h"
#include <box2d/box2d.h>
#include <box2d/types.h>
#include "../../Helpers/Collision.h"
#include "../../Helpers/Core/Error.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Actor.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Level.h"

#define TRIGGER_INPUT_FORCE_TRIGGER 1
#define TRIGGER_INPUT_ENABLE 2
#define TRIGGER_INPUT_DISABLE 3

#define TRIGGER_OUTPUT_TRIGGERED 2
#define TRIGGER_OUTPUT_ENTERED 3
#define TRIGGER_OUTPUT_EXITED 4

typedef struct TriggerData
{
	float width;
	float depth;
	bool oneShot;
	bool enabled;
	bool playerIsColliding;
	bool oneShotHasBeenFired;
	b2ShapeId shape;
} TriggerData;

bool TriggerSignalHandler(Actor *this, const Actor *sender, const byte signal, const Param *param)
{
	TriggerData *data = (TriggerData *)this->extraData;
	if (DefaultSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	if (signal == TRIGGER_INPUT_FORCE_TRIGGER)
	{
		ActorFireOutput(this, TRIGGER_OUTPUT_TRIGGERED, PARAM_NONE);
		return true;
	}
	if (signal == TRIGGER_INPUT_ENABLE && !data->enabled)
	{
		data->enabled = true;
		return true;
	}
	if (signal == TRIGGER_INPUT_DISABLE && data->enabled)
	{
		data->enabled = false;
		return true;
	}
	return false;
}

void CreateTriggerSensor(Actor *trigger, const Vector2 position, const float rotation, const b2WorldId worldId)
{
	TriggerData *data = trigger->extraData;
	b2BodyDef sensorBodyDef = b2DefaultBodyDef();
	sensorBodyDef.type = b2_staticBody;
	sensorBodyDef.position = position;
	const b2BodyId bodyId = b2CreateBody(worldId, &sensorBodyDef);
	const b2Polygon sensorShape = b2MakeOffsetBox(data->width * 0.5f, data->depth * 0.5f, (Vector2){0, 0}, rotation);
	b2ShapeDef sensorShapeDef = b2DefaultShapeDef();
	sensorShapeDef.isSensor = true;
	sensorShapeDef.filter.categoryBits = COLLISION_GROUP_TRIGGER;
	sensorShapeDef.filter.maskBits = COLLISION_GROUP_PLAYER;
	data->shape = b2CreatePolygonShape(bodyId, &sensorShapeDef, &sensorShape);
	trigger->bodyId = bodyId;
}

void TriggerInit(Actor *this, const b2WorldId worldId, const KvList *params)
{
	this->showShadow = false;
	this->extraData = malloc(sizeof(TriggerData));
	CheckAlloc(this->extraData);
	TriggerData *data = this->extraData;
	data->width = KvGetFloat(params, "width", 1.0f);
	data->depth = KvGetFloat(params, "depth", 1.0f);
	data->oneShot = KvGetBool(params, "oneShot", true);
	data->enabled = KvGetBool(params, "startEnabled", true);
	data->playerIsColliding = false;
	data->oneShotHasBeenFired = false;
	CreateTriggerSensor(this, this->position, this->rotation, worldId);
	this->SignalHandler = TriggerSignalHandler;
}

void TriggerUpdate(Actor *this, double /*delta*/)
{
	TriggerData *data = this->extraData;
	if (data->enabled)
	{
		if (GetSensorState(GetState()->level->worldId, data->shape.index1, data->playerIsColliding))
		{
			if (!data->playerIsColliding)
			{
				ActorFireOutput(this, TRIGGER_OUTPUT_ENTERED, PARAM_NONE);
				data->playerIsColliding = true;
			}
			if (!data->oneShotHasBeenFired)
			{
				ActorFireOutput(this, TRIGGER_OUTPUT_TRIGGERED, PARAM_NONE);
				data->oneShotHasBeenFired = data->oneShot;
			}
		} else if (data->playerIsColliding)
		{
			ActorFireOutput(this, TRIGGER_OUTPUT_EXITED, PARAM_NONE);
			data->playerIsColliding = false;
			if (data->oneShotHasBeenFired)
			{
				RemoveActor(this);
			}
		}
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void TriggerDestroy(Actor *this)
{
	b2DestroyBody(this->bodyId);
	((TriggerData *)this->extraData)->shape = b2_nullShapeId;
	free(this->extraData);
	this->extraData = NULL;
}
