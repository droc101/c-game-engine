//
// Created by droc101 on 7/22/25.
//

#include "LogicDecimal.h"
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/Error.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Helpers/Core/Logging.h"
#include "../../Structs/Actor.h"

typedef enum LogicDecimalOp
{
	DECIMAL_OP_EQUAL,
	DECIMAL_OP_GREATER_THAN,
	DECIMAL_OP_GREATER_THAN_OR_EQUAL,
	DECIMAL_OP_LESS_THAN,
	DECIMAL_OP_LESS_THAN_OR_EQUAL
} LogicDecimalOp;

typedef struct LogicDecimalData
{
	float operandA;
	float operandB;
	LogicDecimalOp operation;
} LogicDecimalData;

#define LOGIC_DECIMAL_INPUT_OPERAND_A 1
#define LOGIC_DECIMAL_INPUT_OPERAND_B 2
#define LOGIC_DECIMAL_INPUT_EXECUTE 3

#define LOGIC_DECIMAL_OUTPUT_ON_TRUE 2
#define LOGIC_DECIMAL_OUTPUT_ON_FALSE 3
#define LOGIC_DECIMAL_OUTPUT_EXECUTION_RESULT 4

bool LogicDecimalSignalHandler(Actor *this, const Actor *sender, const byte signal, const Param *param)
{
	LogicDecimalData *data = (LogicDecimalData *)this->extraData;
	if (DefaultSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	if (signal == LOGIC_DECIMAL_INPUT_OPERAND_A)
	{
		if (param->type == PARAM_TYPE_FLOAT)
		{
			data->operandA = param->floatValue;
			return true;
		}
	} else if (signal == LOGIC_DECIMAL_INPUT_OPERAND_B)
	{
		if (param->type == PARAM_TYPE_FLOAT)
		{
			data->operandB = param->floatValue;
			return true;
		}
	} else if (signal == LOGIC_DECIMAL_INPUT_EXECUTE)
	{
		bool result = false;
		switch (data->operation)
		{
			case DECIMAL_OP_EQUAL:
				result = data->operandA == data->operandB;
				break;
			case DECIMAL_OP_GREATER_THAN:
				result = data->operandA > data->operandB;
				break;
			case DECIMAL_OP_GREATER_THAN_OR_EQUAL:
				result = data->operandA >= data->operandB;
				break;
			case DECIMAL_OP_LESS_THAN:
				result = data->operandA < data->operandB;
				break;
			case DECIMAL_OP_LESS_THAN_OR_EQUAL:
				result = data->operandA <= data->operandB;
				break;
			default:
				LogError("Unknown decimal operation: %d", data->operation);
				return false;
		}
		if (result)
		{
			ActorFireOutput(this, LOGIC_DECIMAL_OUTPUT_ON_TRUE, PARAM_NONE);
		} else
		{
			ActorFireOutput(this, LOGIC_DECIMAL_OUTPUT_ON_FALSE, PARAM_NONE);
		}
		ActorFireOutput(this, LOGIC_DECIMAL_OUTPUT_EXECUTION_RESULT, PARAM_BOOL(result));
		return true;
	}
	return false;
}

void LogicDecimalInit(Actor *this, const b2WorldId /*worldId*/, const KvList *params)
{
	LogicDecimalData *data = malloc(sizeof(LogicDecimalData));
	CheckAlloc(data);
	data->operandA = KvGetFloat(params, "operandA", .0f);
	data->operandB = KvGetFloat(params, "operandB", .0f);
	data->operation = KvGetByte(params, "operation", DECIMAL_OP_EQUAL);
	this->SignalHandler = LogicDecimalSignalHandler;
}

void LogicDecimalDestroy(Actor *this)
{
	free(this->extraData);
}
