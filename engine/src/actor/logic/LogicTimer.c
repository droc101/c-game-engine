//
// Created by droc101 on 10/4/26.
//

#include <engine/actor/logic/LogicTimer.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Timing.h>
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct LogicTimerData
{
	bool running;
	uint64_t intervalMsec;
	uint64_t fireTime;
} LogicTimerData;

static void LogicTimerStopHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	LogicTimerData *data = (LogicTimerData *)this->extraData;
	data->running = false;
}

static void LogicTimerStartHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	LogicTimerData *data = (LogicTimerData *)this->extraData;
	data->fireTime = GetTimeMs() + data->intervalMsec;
	data->running = true;
}

static void LogicTimerFireHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	ActorFireOutput(this, LOGIC_TIMER_OUTPUT_ON_TIMER_EXPIRED, PARAM_NONE);
}

static void LogicTimerInit(Actor *this, const KvList params, const Transform * /*transform*/)
{
	this->extraData = malloc(sizeof(LogicTimerData));
	CheckAlloc(this->extraData);
	LogicTimerData *data = this->extraData;
	data->intervalMsec = KvGetUint64(params, "interval_msec", 1000);
	data->running = KvGetBool(params, "autostart", true);
}

static void LogicTimerUpdate(Actor *this, const double /*delta*/)
{
	LogicTimerData *data = this->extraData;
	if (data->running)
	{
		if (GetTimeMs() >= data->fireTime)
		{
			ActorFireOutput(this, LOGIC_TIMER_OUTPUT_ON_TIMER_EXPIRED, PARAM_NONE);
			const uint64_t timeSinceExpiry = GetTimeMs() - data->fireTime;
			data->fireTime = GetTimeMs() + (data->intervalMsec - timeSinceExpiry);
		}
	}
}

ActorDefinition logicTimerActorDefinition = {
	.Init = LogicTimerInit,
	.Update = LogicTimerUpdate,
};

void RegisterLogicTimer()
{
	RegisterDefaultActorInputs(&logicTimerActorDefinition);
	RegisterActorInput(&logicTimerActorDefinition, LOGIC_TIMER_INPUT_START, LogicTimerStartHandler);
	RegisterActorInput(&logicTimerActorDefinition, LOGIC_TIMER_INPUT_STOP, LogicTimerStopHandler);
	RegisterActorInput(&logicTimerActorDefinition, LOGIC_TIMER_INPUT_FIRE, LogicTimerFireHandler);
	RegisterActor(LOGIC_TIMER_ACTOR_NAME, &logicTimerActorDefinition);
}
