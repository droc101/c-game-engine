//
// Created by droc101 on 10/4/26.
//

#include <engine/actor/logic/LogicRandom.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct LogicRandomData
{
	int maxValue;
} LogicRandomData;

static int GetRandomValue(const int maxBound)
{
	return abs(rand()) % maxBound;
}

static void LogicRandomGetRandomNumberHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const LogicRandomData *data = (LogicRandomData *)this->extraData;
	ActorFireOutput(this, LOGIC_RANDOM_OUTPUT_RANDOM_VALUE, PARAM_INT(GetRandomValue(data->maxValue)));
}

static void LogicRandomTriggerHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const LogicRandomData *data = (LogicRandomData *)this->extraData;
	const int value = GetRandomValue(data->maxValue);
	if (value > 15)
	{
		LogError("logic_random: case greater than 15, not firing\n");
		return;
	}
	const char *outputs[16] = {
		LOGIC_RANDOM_OUTPUT_ON_VALUE_ONE,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_TWO,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_THREE,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_FOUR,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_FIVE,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_SIX,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_SEVEN,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_EIGHT,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_NINE,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_TEN,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_ELEVEN,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_TWELVE,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_THIRTEEN,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_FOURTEEN,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_FIFTEEN,
		LOGIC_RANDOM_OUTPUT_ON_VALUE_SIXTEEN,
	};
	ActorFireOutput(this, outputs[value-1], PARAM_NONE);
}

static void LogicRandomInit(Actor *this, const KvList params, const Transform * /*transform*/)
{
	this->extraData = malloc(sizeof(LogicRandomData));
	CheckAlloc(this->extraData);
	LogicRandomData *data = this->extraData;
	data->maxValue = KvGetInt(params, "max_value", 16);
}

ActorDefinition logicRandomActorDefinition = {
	.Init = LogicRandomInit,
};

void RegisterLogicRandom()
{
	RegisterDefaultActorInputs(&logicRandomActorDefinition);
	RegisterActorInput(&logicRandomActorDefinition,
					   LOGIC_RANDOM_INPUT_GET_RANDOM_NUMBER,
					   LogicRandomGetRandomNumberHandler);
	RegisterActorInput(&logicRandomActorDefinition, LOGIC_RANDOM_INPUT_TRIGGER, LogicRandomTriggerHandler);
	RegisterActor(LOGIC_RANDOM_ACTOR_NAME, &logicRandomActorDefinition);
}
