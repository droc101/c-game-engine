//
// Created by droc101 on 6/19/25.
//

#include "LogicBinary.h"
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/Error.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Helpers/Core/Logging.h"
#include "../../Structs/Actor.h"

typedef enum LogicBinaryOp
{
	BINARY_OP_AND,
	BINARY_OP_OR,
	UNARY_OP_NOT,
} LogicOp;

typedef struct LogicBinaryData
{
	bool operandA;
	bool operandB;
	LogicOp operation;
} LogicBinaryData;

#define LOGIC_BINARY_INPUT_OPERAND_A 1
#define LOGIC_BINARY_INPUT_OPERAND_B 2
#define LOGIC_BINARY_INPUT_EXECUTE 3

#define LOGIC_BINARY_OUTPUT_ON_TRUE 2
#define LOGIC_BINARY_OUTPUT_ON_FALSE 3
#define LOGIC_BINARY_OUTPUT_EXECUTION_RESULT 4

bool LogicBinarySignalHandler(Actor *this, const Actor *sender, const byte signal, const Param *param)
{
	LogicBinaryData *data = (LogicBinaryData *)this->extraData;
	if (DefaultSignalHandler(this, sender, signal, param))
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
			case BINARY_OP_AND:
				result = data->operandA && data->operandB;
				break;
			case BINARY_OP_OR:
				result = data->operandA || data->operandB;
				break;
			case UNARY_OP_NOT:
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

void LogicBinaryInit(Actor *this, const KvList *params)
{
	this->extraData = malloc(sizeof(LogicBinaryData));
	CheckAlloc(this->extraData);
	LogicBinaryData *data = this->extraData;
	data->operandA = false;
	data->operandB = false;
	data->operation = KvGetByte(params, "operation", BINARY_OP_AND);
	this->SignalHandler = LogicBinarySignalHandler;
}
