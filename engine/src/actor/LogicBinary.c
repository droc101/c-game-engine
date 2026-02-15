//
// Created by droc101 on 6/19/25.
//

#include <engine/actor/LogicBinary.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stdlib.h>

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

static void LogicBinaryOperandAHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	LogicBinaryData *data = (LogicBinaryData *)this->extraData;
	if (param->type == PARAM_TYPE_BOOL)
	{
		data->operandA = param->boolValue;
	}
}

static void LogicBinaryOperandBHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	LogicBinaryData *data = (LogicBinaryData *)this->extraData;
	if (param->type == PARAM_TYPE_BOOL)
	{
		data->operandB = param->boolValue;
	}
}

static void LogicBinaryExecuteHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const LogicBinaryData *data = (LogicBinaryData *)this->extraData;
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
			return;
	}
	if (result)
	{
		ActorFireOutput(this, LOGIC_BINARY_OUTPUT_ON_TRUE, PARAM_NONE);
	} else
	{
		ActorFireOutput(this, LOGIC_BINARY_OUTPUT_ON_FALSE, PARAM_NONE);
	}
	ActorFireOutput(this, LOGIC_BINARY_OUTPUT_EXECUTION_RESULT, PARAM_BOOL(result));
}

void LogicBinaryInit(Actor *this, const KvList params, Transform * /*transform*/)
{
	this->extraData = malloc(sizeof(LogicBinaryData));
	CheckAlloc(this->extraData);
	LogicBinaryData *data = this->extraData;
	data->operandA = false;
	data->operandB = false;
	data->operation = KvGetByte(params, "operation", LOGIC_OP_AND);
}

ActorDefinition logicBinaryActorDefinition = {
	.Update = DefaultActorUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = LogicBinaryInit,
};

void RegisterLogicBinary()
{
	RegisterDefaultActorInputs(&logicBinaryActorDefinition);
	RegisterActorInput(&logicBinaryActorDefinition, LOGIC_BINARY_INPUT_OPERAND_A, LogicBinaryOperandAHandler);
	RegisterActorInput(&logicBinaryActorDefinition, LOGIC_BINARY_INPUT_OPERAND_B, LogicBinaryOperandBHandler);
	RegisterActorInput(&logicBinaryActorDefinition, LOGIC_BINARY_INPUT_EXECUTE, LogicBinaryExecuteHandler);
	RegisterActor(LOGIC_BINARY_ACTOR_NAME, &logicBinaryActorDefinition);
}
