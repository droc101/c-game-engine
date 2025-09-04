//
// Created by droc101 on 6/19/25.
//

#include "LogicBinary.h"
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../Helpers/Core/Error.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Helpers/Core/Logging.h"
#include "../../Structs/Actor.h"
#include "../../Structs/ActorDefinition.h"

enum LogicBinaryInput
{
	LOGIC_BINARY_INPUT_OPERAND_A = 1,
	LOGIC_BINARY_INPUT_OPERAND_B = 2,
	LOGIC_BINARY_INPUT_EXECUTE = 3,
};

enum LogicBinaryOutput
{
	LOGIC_BINARY_OUTPUT_ON_TRUE = 2,
	LOGIC_BINARY_OUTPUT_ON_FALSE = 3,
	LOGIC_BINARY_OUTPUT_EXECUTION_RESULT = 4,
};

typedef enum LogicOp
{
	LOGIC_OP_AND,
	LOGIC_OP_OR,
	LOGIC_OP_NOT,
} LogicOp;

typedef struct LogicBinaryData
{
	bool operandA;
	bool operandB;
	LogicOp operation;
} LogicBinaryData;

static bool LogicBinarySignalHandler(Actor *this, const Actor *sender, const uint8_t signal, const Param *param)
{
	LogicBinaryData *data = (LogicBinaryData *)this->extraData;
	if (DefaultActorSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	if (signal == LOGIC_BINARY_INPUT_OPERAND_A)
	{
		if (param->type == PARAM_TYPE_BOOL)
		{
			data->operandA = param->boolValue;
			return true;
		}
	} else if (signal == LOGIC_BINARY_INPUT_OPERAND_B)
	{
		if (param->type == PARAM_TYPE_BOOL)
		{
			data->operandB = param->boolValue;
			return true;
		}
	} else if (signal == LOGIC_BINARY_INPUT_EXECUTE)
	{
		bool result = false;
		switch (data->operation)
		{
			case LOGIC_OP_AND:
				result = data->operandA && data->operandB;
				break;
			case LOGIC_OP_OR:
				result = data->operandA || data->operandB;
				break;
			case LOGIC_OP_NOT:
				result = !data->operandA;
				break;
			default:
				LogError("Unknown binary operation: %d", data->operation);
				return false;
		}
		if (result)
		{
			ActorFireOutput(this, LOGIC_BINARY_OUTPUT_ON_TRUE, PARAM_NONE);
		} else
		{
			ActorFireOutput(this, LOGIC_BINARY_OUTPUT_ON_FALSE, PARAM_NONE);
		}
		ActorFireOutput(this, LOGIC_BINARY_OUTPUT_EXECUTION_RESULT, PARAM_BOOL(result));
		return true;
	}
	return false;
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_LOGIC_BINARY,
	.Update = DefaultActorUpdate,
	.SignalHandler = LogicBinarySignalHandler,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
};

void LogicBinaryInit(Actor *this, const KvList params, Transform * /*transform*/)
{
	this->definition = &definition;

	this->extraData = malloc(sizeof(LogicBinaryData));
	CheckAlloc(this->extraData);
	LogicBinaryData *data = this->extraData;
	data->operandA = false;
	data->operandB = false;
	data->operation = KvGetByte(params, "operation", LOGIC_OP_AND);
}
