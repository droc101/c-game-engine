//
// Created by droc101 on 7/22/25.
//

#include "LogicCounter.h"
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../Helpers/Core/Error.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Helpers/Core/MathEx.h"
#include "../../Structs/Actor.h"
#include "../../Structs/ActorDefinition.h"

enum LogicCounterInput
{
	LOGIC_COUNTER_INPUT_INCREMENT = 1,
	LOGIC_COUNTER_INPUT_DECREMENT = 2,
	LOGIC_COUNTER_INPUT_ADD = 3,
	LOGIC_COUNTER_INPUT_SUBTRACT = 4,
};

enum LogicCounterOutput
{
	LOGIC_COUNTER_OUTPUT_HIT_MAX = 2,
	LOGIC_COUNTER_OUTPUT_HIT_MIN = 3,
	LOGIC_COUNTER_OUTPUT_LEFT_MAX = 4,
	LOGIC_COUNTER_OUTPUT_LEFT_MIN = 5,
	LOGIC_COUNTER_OUTPUT_COUNTER_CHANGED = 6,
};

typedef struct LogicCounterData
{
	int counter;
	int min;
	int max;
	bool clampToMin;
	bool clampToMax;
} LogicCounterData;

static inline void ChangeValue(const int change, LogicCounterData *data, const Actor *this)
{
	const int prevValue = data->counter;
	data->counter += change;
	if (data->clampToMax)
	{
		if (data->counter > data->max)
		{
			data->counter = data->max;
		}
		if (prevValue < data->max && data->counter == data->max)
		{
			ActorFireOutput(this, LOGIC_COUNTER_OUTPUT_HIT_MAX, PARAM_NONE);
		} else if (prevValue == data->max && data->counter < data->max)
		{
			ActorFireOutput(this, LOGIC_COUNTER_OUTPUT_LEFT_MAX, PARAM_NONE);
		}
	}
	if (data->clampToMin)
	{
		if (data->counter < data->min)
		{
			data->counter = data->min;
		}
		if (prevValue > data->min && data->counter == data->min)
		{
			ActorFireOutput(this, LOGIC_COUNTER_OUTPUT_HIT_MIN, PARAM_NONE);
		} else if (prevValue == data->min && data->counter > data->min)
		{
			ActorFireOutput(this, LOGIC_COUNTER_OUTPUT_LEFT_MIN, PARAM_NONE);
		}
	}
	if (prevValue != data->counter)
	{
		ActorFireOutput(this, LOGIC_COUNTER_OUTPUT_COUNTER_CHANGED, PARAM_INT(data->counter));
	}
}

static bool LogicCounterSignalHandler(Actor *this, const Actor *sender, const uint8_t signal, const Param *param)
{
	LogicCounterData *data = (LogicCounterData *)this->extraData;
	if (DefaultActorSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	if (signal == LOGIC_COUNTER_INPUT_ADD)
	{
		if (param->type == PARAM_TYPE_INTEGER)
		{
			ChangeValue(param->intValue, data, this);
			return true;
		}
	} else if (signal == LOGIC_COUNTER_INPUT_SUBTRACT)
	{
		if (param->type == PARAM_TYPE_INTEGER)
		{
			ChangeValue(-param->intValue, data, this);
			return true;
		}
	} else if (signal == LOGIC_COUNTER_INPUT_INCREMENT)
	{
		ChangeValue(1, data, this);
		return true;
	} else if (signal == LOGIC_COUNTER_INPUT_DECREMENT)
	{
		ChangeValue(-1, data, this);
		return true;
	}
	return false;
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_LOGIC_COUNTER,
	.Update = DefaultActorUpdate,
	.SignalHandler = LogicCounterSignalHandler,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
};

void LogicCounterInit(Actor *this, const KvList params, Transform * /*transform*/)
{
	this->definition = &definition;

	this->extraData = malloc(sizeof(LogicCounterData));
	CheckAlloc(this->extraData);
	LogicCounterData *data = this->extraData;
	data->min = KvGetInt(params, "min", 0);
	data->max = KvGetInt(params, "max", 100);
	data->counter = KvGetInt(params, "counter", 0);
	data->counter = clamp(data->counter, data->min, data->max);
	data->clampToMax = KvGetBool(params, "clampToMax", true);
	data->clampToMin = KvGetBool(params, "clampToMin", true);
}
