//
// Created by droc101 on 7/22/25.
//

#include <engine/actor/LogicDecimal.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stdlib.h>

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

static void LogicDecimalOperandAHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	LogicDecimalData *data = (LogicDecimalData *)this->extraData;
	if (param->type == PARAM_TYPE_FLOAT)
	{
		data->operandA = param->floatValue;
	}
}

static void LogicDecimalOperandBHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	LogicDecimalData *data = (LogicDecimalData *)this->extraData;
	if (param->type == PARAM_TYPE_FLOAT)
	{
		data->operandB = param->floatValue;
	}
}

static void LogicDecimalExecuteHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const LogicDecimalData *data = (LogicDecimalData *)this->extraData;
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
			return;
	}
	if (result)
	{
		ActorFireOutput(this, LOGIC_DECIMAL_OUTPUT_ON_TRUE, PARAM_NONE);
	} else
	{
		ActorFireOutput(this, LOGIC_DECIMAL_OUTPUT_ON_FALSE, PARAM_NONE);
	}
	ActorFireOutput(this, LOGIC_DECIMAL_OUTPUT_EXECUTION_RESULT, PARAM_BOOL(result));
}

void LogicDecimalInit(Actor *this, const KvList params, Transform * /*transform*/)
{
	this->extraData = malloc(sizeof(LogicDecimalData));
	CheckAlloc(this->extraData);
	LogicDecimalData *data = this->extraData;
	data->operandA = KvGetFloat(params, "operandA", .0f);
	data->operandB = KvGetFloat(params, "operandB", .0f);
	data->operation = KvGetByte(params, "operation", DECIMAL_OP_EQUAL);
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_LOGIC_DECIMAL,
	.Update = DefaultActorUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = LogicDecimalInit,
};

void RegisterLogicDecimal()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActorInput(&definition, LOGIC_DECIMAL_INPUT_OPERAND_A, LogicDecimalOperandAHandler);
	RegisterActorInput(&definition, LOGIC_DECIMAL_INPUT_OPERAND_B, LogicDecimalOperandBHandler);
	RegisterActorInput(&definition, LOGIC_DECIMAL_INPUT_EXECUTE, LogicDecimalExecuteHandler);
	RegisterActor(LOGIC_DECIMAL_ACTOR_NAME, &definition);
}
