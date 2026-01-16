//
// Created by droc101 on 7/22/25.
//

#include <engine/actor/LogicCounter.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stdlib.h>

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

static void LogicCounterAddHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	LogicCounterData *data = (LogicCounterData *)this->extraData;
	if (param->type == PARAM_TYPE_INTEGER)
	{
		ChangeValue(param->intValue, data, this);
	}
}

static void LogicCounterSubtractHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	LogicCounterData *data = (LogicCounterData *)this->extraData;
	if (param->type == PARAM_TYPE_INTEGER)
	{
		ChangeValue(-param->intValue, data, this);
	}
}

static void LogicCounterIncrementHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	LogicCounterData *data = (LogicCounterData *)this->extraData;
	ChangeValue(1, data, this);
}

static void LogicCounterDecrementHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	LogicCounterData *data = (LogicCounterData *)this->extraData;
	ChangeValue(1, data, this);
}

void LogicCounterInit(Actor *this, const KvList params, Transform * /*transform*/)
{
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

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_LOGIC_COUNTER,
	.Update = DefaultActorUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = LogicCounterInit,
};

void RegisterLogicCounter()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActorInput(&definition, LOGIC_COUNTER_INPUT_ADD, LogicCounterAddHandler);
	RegisterActorInput(&definition, LOGIC_COUNTER_INPUT_SUBTRACT, LogicCounterSubtractHandler);
	RegisterActorInput(&definition, LOGIC_COUNTER_INPUT_INCREMENT, LogicCounterIncrementHandler);
	RegisterActorInput(&definition, LOGIC_COUNTER_INPUT_DECREMENT, LogicCounterDecrementHandler);
	RegisterActor(LOGIC_COUNTER_ACTOR_NAME, &definition);
}
