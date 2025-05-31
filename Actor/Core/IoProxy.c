//
// Created by droc101 on 4/21/25.
//

#include "IoProxy.h"
#include "../../Helpers/Core/Logging.h"
#include "../../Structs/Actor.h"
#include "../../Structs/GlobalState.h"

#define IOPROXY_OUTPUT_FIRST_TICK 2

bool IoProxySignalHandler(Actor *self, const Actor *sender, byte signal, const Param *param)
{
	if (signal == ACTOR_KILL_INPUT)
	{
		LogError("IoProxy actor should not be killed! The kill input will be ignored!");
		return false;
	}
	if (DefaultSignalHandler(self, sender, signal, param))
	{
		return true;
	}
	return false;
}

void IoProxyInit(Actor *this, const b2WorldId worldId, KvList *params)
{
	if (GetState()->level->ioProxy != NULL)
	{
		LogError("Attempted to add an I/O proxy actor to level, but it already has one! The new one cannot be used.");
	} else
	{
		GetState()->level->ioProxy = this;
	}
	this->showShadow = false;
	this->SignalHandler = IoProxySignalHandler;
}

void IoProxyUpdate(Actor *this, double /*delta*/)
{
	if (GetState()->physicsFrame == 1)
	{
		ActorFireOutput(this, IOPROXY_OUTPUT_FIRST_TICK, PARAM_NONE);
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void IoProxyDestroy(Actor *this) {}
