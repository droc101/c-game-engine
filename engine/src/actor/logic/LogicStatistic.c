//
// Created by droc101 on 10/4/26.
//

#include <engine/actor/logic/LogicStatistic.h>
#include <engine/structs/Achievements.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <joltc/Math/Transform.h>
#include <stdlib.h>
#include <string.h>

typedef struct LogicStatisticData
{
	char *statName;
	StatType statType;
} LogicStatisticData;

static void LogicStatisticAddHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	int intAddValue = 1;
	float floatAddValue = 1;
	switch (param->type)
	{
		case PARAM_TYPE_BYTE:
			intAddValue = param->byteValue;
			floatAddValue = param->byteValue;
			break;
		case PARAM_TYPE_INTEGER:
			intAddValue = param->intValue;
			floatAddValue = (float)param->intValue;
			break;
		case PARAM_TYPE_FLOAT:
			intAddValue = (int)param->floatValue;
			floatAddValue = param->floatValue;
			break;
		case PARAM_TYPE_UINT_64:
			intAddValue = (int)param->uint64value;
			floatAddValue = (float)param->uint64value;
			break;
		default:
			break;
	}

	const LogicStatisticData *data = (LogicStatisticData *)this->extraData;
	if (data->statType == STAT_TYPE_INT)
	{
		IncrementIntegerStatistic(data->statName, intAddValue);
	} else if (data->statType == STAT_TYPE_FLOAT)
	{
		IncrementFloatStatistic(data->statName, floatAddValue);
	}
}

static void LogicStatisticSetHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	int intValue = 0;
	float floatValue = 0;
	switch (param->type)
	{
		case PARAM_TYPE_BYTE:
			intValue = param->byteValue;
			floatValue = param->byteValue;
			break;
		case PARAM_TYPE_INTEGER:
			intValue = param->intValue;
			floatValue = (float)param->intValue;
			break;
		case PARAM_TYPE_FLOAT:
			intValue = (int)param->floatValue;
			floatValue = param->floatValue;
			break;
		case PARAM_TYPE_UINT_64:
			intValue = (int)param->uint64value;
			floatValue = (float)param->uint64value;
			break;
		default:
			break;
	}

	const LogicStatisticData *data = (LogicStatisticData *)this->extraData;
	if (data->statType == STAT_TYPE_INT)
	{
		SetIntegerStatistic(data->statName, intValue);
	} else if (data->statType == STAT_TYPE_FLOAT)
	{
		SetFloatStatistic(data->statName, floatValue);
	}
}

static void LogicStatisticInit(Actor *this, const KvList params, const Transform * /*transform*/)
{
	this->extraData = malloc(sizeof(LogicStatisticData));
	CheckAlloc(this->extraData);
	LogicStatisticData *data = this->extraData;
	data->statName = strdup(KvGetString(params, "statistic", ""));
	data->statType = GetStatisticType(data->statName);
}

static void LogicStatisticDestroy(Actor *this)
{
	const LogicStatisticData *data = this->extraData;
	free(data->statName);
}

ActorDefinition logicStatisticActorDefinition = {
	.Init = LogicStatisticInit,
	.Destroy = LogicStatisticDestroy,
};

void RegisterLogicStatistic()
{
	RegisterDefaultActorInputs(&logicStatisticActorDefinition);
	RegisterActorInput(&logicStatisticActorDefinition, LOGIC_STATISTIC_INPUT_ADD, LogicStatisticAddHandler);
	RegisterActorInput(&logicStatisticActorDefinition, LOGIC_STATISTIC_INPUT_SET, LogicStatisticSetHandler);
	RegisterActor(LOGIC_STATISTIC_ACTOR_NAME, &logicStatisticActorDefinition);
}
