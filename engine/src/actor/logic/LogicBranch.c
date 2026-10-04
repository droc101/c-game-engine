//
// Created by droc101 on 10/4/26.
//

#include <engine/actor/logic/LogicBranch.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stdlib.h>

static void LogicBranchSwitchHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	int value = 1;
	switch (param->type)
	{
		case PARAM_TYPE_BYTE:
			value = param->byteValue;
			break;
		case PARAM_TYPE_INTEGER:
			value = param->intValue;
			break;
		case PARAM_TYPE_FLOAT:
			value = (int)param->floatValue;
			break;
		case PARAM_TYPE_UINT_64:
			value = (int)param->uint64value;
			break;
		default:
			break;
	}

	if (value > 15)
	{
		LogError("logic_branch: case greater than 15, not firing\n");
		return;
	}
	const char *outputs[16] = {
		LOGIC_BRANCH_OUTPUT_ON_VALUE_ONE,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_TWO,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_THREE,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_FOUR,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_FIVE,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_SIX,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_SEVEN,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_EIGHT,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_NINE,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_TEN,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_ELEVEN,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_TWELVE,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_THIRTEEN,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_FOURTEEN,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_FIFTEEN,
		LOGIC_BRANCH_OUTPUT_ON_VALUE_SIXTEEN,
	};
	ActorFireOutput(this, outputs[value], PARAM_NONE);
}

static void LogicBranchInit(Actor */*this*/, const KvList /*params*/, const Transform * /*transform*/) {}

ActorDefinition logicBranchActorDefinition = {
	.Init = LogicBranchInit,
};

void RegisterLogicBranch()
{
	RegisterDefaultActorInputs(&logicBranchActorDefinition);
	RegisterActorInput(&logicBranchActorDefinition, LOGIC_BRANCH_INPUT_SWITCH, LogicBranchSwitchHandler);
	RegisterActor(LOGIC_BRANCH_ACTOR_NAME, &logicBranchActorDefinition);
}
