//
// Created by droc101 on 10/4/26.
//

#include <engine/actor/logic/LogicCast.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stdlib.h>

static void LogicCastCastHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	bool boolValue = false;
	uint8_t byteValue = 0;
	int intValue = 0;
	float floatValue = 0;
	uint64_t uint64Value = 0;

	switch (param->type)
	{
		case PARAM_TYPE_BYTE:
			boolValue = param->byteValue != 0;
			byteValue = param->byteValue;
			intValue = param->byteValue;
			floatValue = param->byteValue;
			uint64Value = param->byteValue;
			break;
		case PARAM_TYPE_INTEGER:
			boolValue = param->intValue != 0;
			byteValue = param->intValue;
			intValue = param->intValue;
			floatValue = (float)param->intValue;
			uint64Value = param->intValue;
			break;
		case PARAM_TYPE_FLOAT:
			boolValue = param->floatValue != 0;
			byteValue = (uint8_t)param->floatValue;
			intValue = (int32_t)param->floatValue;
			floatValue = param->floatValue;
			uint64Value = (uint64_t)param->floatValue;
			break;
		case PARAM_TYPE_BOOL:
			boolValue = param->boolValue;
			byteValue = param->boolValue ? 0 : 1;
			intValue = param->boolValue ? 0 : 1;
			floatValue = param->boolValue ? 0 : 1;
			uint64Value = param->boolValue ? 0 : 1;
			break;
		case PARAM_TYPE_UINT_64:
			boolValue = param->uint64value != 0;
			byteValue = param->uint64value;
			intValue = (int)param->uint64value;
			floatValue = (float)param->uint64value;
			uint64Value = param->uint64value;
			break;
		default:
			LogError("logic_cast: uncastable type\n");
			return;
	}

	ActorFireOutput(this, LOGIC_CAST_OUTPUT_ON_CAST_BOOL, PARAM_BOOL(boolValue));
	ActorFireOutput(this, LOGIC_CAST_OUTPUT_ON_CAST_BYTE, PARAM_BYTE(byteValue));
	ActorFireOutput(this, LOGIC_CAST_OUTPUT_ON_CAST_INT, PARAM_INT(intValue));
	ActorFireOutput(this, LOGIC_CAST_OUTPUT_ON_CAST_FLOAT, PARAM_FLOAT(floatValue));
	ActorFireOutput(this, LOGIC_CAST_OUTPUT_ON_CAST_SIZE_T, PARAM_UINT_64(uint64Value));
}

static void LogicCastInit(Actor */*this*/, const KvList /*params*/, const Transform * /*transform*/) {}

ActorDefinition logicCastActorDefinition = {
	.Init = LogicCastInit,
};

void RegisterLogicCast()
{
	RegisterDefaultActorInputs(&logicCastActorDefinition);
	RegisterActorInput(&logicCastActorDefinition, LOGIC_CAST_INPUT_CAST, LogicCastCastHandler);
	RegisterActor(LOGIC_CAST_ACTOR_NAME, &logicCastActorDefinition);
}
