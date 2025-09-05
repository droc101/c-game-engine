//
// Created by droc101 on 4/21/25.
//

#include "IoProxy.h"
#include <joltc/Math/Transform.h>
#include <stddef.h>
#include <stdlib.h>
#include "../../Helpers/Core/KVList.h"
#include "../../Helpers/Core/Logging.h"
#include "../../Structs/Actor.h"
#include "../../Structs/ActorDefinition.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Param.h"

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
		ActorFireOutput(this, IO_PROXY_OUTPUT_FIRST_TICK, PARAM_NONE);
	}
	data->tickCounter++;
}

void IoProxyInit(Actor *this, const KvList /*params*/, Transform * /*transform*/)
{
	if (GetState()->level->ioProxy != NULL)
	{
		LogError("Attempted to add an I/O proxy actor to level, but it already has one! The new one cannot be used.");
	} else
	{
		GetState()->level->ioProxy = this;
	}
	this->extraData = calloc(1, sizeof(IoProxyData));
}

static ActorDefinition definition = {.actorType = ACTOR_TYPE_IO_PROXY,
									 .Update = IoProxyUpdate,
									 .OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
									 .OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
									 .OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
									 .RenderUi = DefaultActorRenderUi,
									 .Destroy = DefaultActorDestroy,
									 .Init = IoProxyInit};

void RegisterIoProxy()
{
	RegisterDefaultActorInputs(&definition);
	UnregisterActorInput(&definition, ACTOR_INPUT_KILL);
	RegisterActor(IO_PROXY_ACTOR_NAME, &definition);
}
