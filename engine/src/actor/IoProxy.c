//
// Created by droc101 on 4/21/25.
//

#include <engine/actor/IoProxy.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <joltc/Math/Transform.h>
#include <stddef.h>
#include <stdlib.h>

static void IoProxyUpdate(Actor *this, double /*delta*/)
{
	if (GetState()->map->physicsTick == 1)
	{
		ActorFireOutput(this, IO_PROXY_OUTPUT_FIRST_TICK, PARAM_NONE);
	}
}

void IoProxyInit(Actor *this, const KvList /*params*/, Transform * /*transform*/)
{
	if (GetState()->map->ioProxy != NULL)
	{
		LogError("Attempted to add an I/O proxy actor to level, but it already has one! The new one cannot be used.");
	} else
	{
		GetState()->map->ioProxy = this;
	}
	CheckAlloc(this->extraData);
}

ActorDefinition ioProxyActorDefinition = {
	.Update = IoProxyUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = DefaultActorDestroy,
	.Init = IoProxyInit,
};

void RegisterIoProxy()
{
	RegisterDefaultActorInputs(&ioProxyActorDefinition);
	UnregisterActorInput(&ioProxyActorDefinition, ACTOR_INPUT_KILL);
	RegisterActor(IO_PROXY_ACTOR_NAME, &ioProxyActorDefinition);
}
