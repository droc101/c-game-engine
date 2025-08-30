//
// Created by droc101 on 4/21/25.
//

#include "IoProxy.h"
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../Helpers/Core/KVList.h"
#include "../../Helpers/Core/Logging.h"
#include "../../Structs/Actor.h"
#include "../../Structs/ActorDefinitions.h"
#include "../../Structs/GlobalState.h"

enum IoProxyOutput
{
	IOPROXY_OUTPUT_FIRST_TICK = 2,
};

typedef struct IoProxyData
{
	/// Tick counter for the whole level so it doesn't get reset when pausing
	size_t tickCounter;
} IoProxyData;

static void IoProxyUpdate(Actor *this, double /*delta*/)
{
	IoProxyData *data = this->extraData;
	if (data->tickCounter == 1)
	{
		ActorFireOutput(this, IOPROXY_OUTPUT_FIRST_TICK, PARAM_NONE);
	}
	data->tickCounter++;
}

static bool IoProxySignalHandler(Actor *this, const Actor *sender, const uint8_t signal, const Param *param)
{
	if (signal == ACTOR_INPUT_KILL)
	{
		LogError("IoProxy actor should not be killed! The kill input will be ignored!");
		return false;
	}
	if (DefaultActorSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	return false;
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_IO_PROXY,
	.Update = IoProxyUpdate,
	.SignalHandler = IoProxySignalHandler,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
};

void IoProxyInit(Actor *this, const KvList * /*params*/, Transform * /*transform*/)
{
	this->definition = &definition;

	if (GetState()->level->ioProxy != NULL)
	{
		LogError("Attempted to add an I/O proxy actor to level, but it already has one! The new one cannot be used.");
	} else
	{
		GetState()->level->ioProxy = this;
	}
	this->extraData = calloc(1, sizeof(IoProxyData));
}
