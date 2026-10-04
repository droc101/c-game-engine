//
// Created by droc101 on 10/4/26.
//

#include <engine/actor/logic/LogicFunction.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct LogicFunctionData
{
	bool enabled;
} LogicFunctionData;

static void LogicFunctionEnableHandler(Actor *this, const Actor * /*sender*/, const Param */*param*/)
{
	LogicFunctionData *data = (LogicFunctionData *)this->extraData;
	data->enabled = true;
}

static void LogicFunctionDisableHandler(Actor *this, const Actor * /*sender*/, const Param */*param*/)
{
	LogicFunctionData *data = (LogicFunctionData *)this->extraData;
	data->enabled = false;
}

static void LogicFunctionRunHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const LogicFunctionData *data = (LogicFunctionData *)this->extraData;
	if (data->enabled)
	{
		ActorFireOutput(this, LOGIC_FUNCTION_OUTPUT_ON_RUN, PARAM_NONE);
	}
}

static void LogicFunctionInit(Actor *this, const KvList params, const Transform * /*transform*/)
{
	this->extraData = malloc(sizeof(LogicFunctionData));
	CheckAlloc(this->extraData);
	LogicFunctionData *data = this->extraData;
	data->enabled = KvGetBool(params, "start_enabled", true);
}

ActorDefinition logicFunctionActorDefinition = {
	.Init = LogicFunctionInit,
};

void RegisterLogicFunction()
{
	RegisterDefaultActorInputs(&logicFunctionActorDefinition);
	RegisterActorInput(&logicFunctionActorDefinition, LOGIC_FUNCTION_INPUT_ENABLE, LogicFunctionEnableHandler);
	RegisterActorInput(&logicFunctionActorDefinition, LOGIC_FUNCTION_INPUT_DISABLE, LogicFunctionDisableHandler);
	RegisterActorInput(&logicFunctionActorDefinition, LOGIC_FUNCTION_INPUT_RUN, LogicFunctionRunHandler);
	RegisterActor(LOGIC_FUNCTION_OUTPUT_ON_RUN, &logicFunctionActorDefinition);
}