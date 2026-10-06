//
// Created by droc101 on 10/4/26.
//

#include <engine/actor/logic/LogicPrint.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/Math/Transform.h>
#include <stdlib.h>
#include <string.h>

typedef struct LogicPrintData
{
	char *prefix;
	char *suffix;
} LogicPrintData;

static void LogicPrintPrintValueHandler(Actor *this, const Actor * /*sender*/, const Param *param)
{
	const LogicPrintData *data = (LogicPrintData *)this->extraData;
	switch (param->type)
	{
		case PARAM_TYPE_BYTE:
			LogInfo("%s%u%s", data->prefix, param->byteValue, data->suffix);
			break;
		case PARAM_TYPE_INTEGER:
			LogInfo("%s%d%s", data->prefix, param->intValue, data->suffix);
			break;
		case PARAM_TYPE_FLOAT:
			LogInfo("%s%f%s", data->prefix, param->floatValue, data->suffix);
			break;
		case PARAM_TYPE_BOOL:
			LogInfo("%s%s%s", data->prefix, param->boolValue ? "true" : "false", data->suffix);
			break;
		case PARAM_TYPE_STRING:
			LogInfo("%s%s%s", data->prefix, param->stringValue, data->suffix);
			break;
		case PARAM_TYPE_NONE:
			LogInfo("%s(none)%s", data->prefix, data->suffix);
			break;
		case PARAM_TYPE_COLOR:
			LogInfo("%sR:%f B:%f, G:%f, A:%f%s",
					data->prefix,
					param->colorValue.r,
					param->colorValue.g,
					param->colorValue.b,
					param->colorValue.a,
					data->suffix);
			break;
		case PARAM_TYPE_KV_LIST:
			LogInfo("%sKvList %zu key(s)%s", data->prefix, KvList_size(param->kvListValue), data->suffix);
			break;
		case PARAM_TYPE_ARRAY:
			LogInfo("%sArray %zu element(s)%s", data->prefix, param->arrayValue.length, data->suffix);
			break;
		case PARAM_TYPE_UINT_64:
			LogInfo("%s%zu%s", data->prefix, param->uint64value, data->suffix);
			break;
		case PARAM_TYPE_VEC2:
			LogInfo("%s(%f, %f)%s", data->prefix, param->vec2value.x, param->vec2value.y, data->suffix);
			break;
		case PARAM_TYPE_VEC3:
			LogInfo("%s(%f, %f, %f)%s",
					data->prefix,
					param->vec3value.x,
					param->vec3value.y,
					param->vec3value.z,
					data->suffix);
			break;
	}
}

static void LogicPrintInit(Actor *this, const KvList params, const Transform * /*transform*/)
{
	this->extraData = malloc(sizeof(LogicPrintData));
	CheckAlloc(this->extraData);
	LogicPrintData *data = this->extraData;
	data->prefix = strdup(KvGetString(params, "prefix", ""));
	data->suffix = strdup(KvGetString(params, "suffix", ""));
}

static void LogicPrintDestroy(Actor *this)
{
	const LogicPrintData *data = this->extraData;
	free(data->prefix);
	free(data->suffix);
}

ActorDefinition logicPrintActorDefinition = {
	.Init = LogicPrintInit,
	.Destroy = LogicPrintDestroy,
};

void RegisterLogicPrint()
{
	RegisterDefaultActorInputs(&logicPrintActorDefinition);
	RegisterActorInput(&logicPrintActorDefinition, LOGIC_PRINT_INPUT_PRINT_VALUE, LogicPrintPrintValueHandler);
	RegisterActor(LOGIC_PRINT_ACTOR_NAME, &logicPrintActorDefinition);
}
