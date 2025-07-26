//
// Created by droc101 on 4/22/2024.
//

#include "Actor.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/Logging.h"
#include "GlobalState.h"
#include "Level.h"

#include "../Actor/Coin.h"
#include "../Actor/Core/IoProxy.h"
#include "../Actor/Core/LogicBinary.h"
#include "../Actor/Core/LogicCounter.h"
#include "../Actor/Core/LogicDecimal.h"
#include "../Actor/Core/SoundPlayer.h"
#include "../Actor/Core/Sprite.h"
#include "../Actor/Core/StaticModel.h"
#include "../Actor/Core/Trigger.h"
#include "../Actor/Door.h"
#include "../Actor/Goal.h"
#include "../Actor/Laser.h"
#include "../Actor/LaserEmitter.h"
#include "../Actor/Physbox.h"
#include "../Actor/TestActor.h"

// Empty template functions
void ActorInit(Actor * /*this*/, const KvList * /*params*/, JPH_BodyInterface * /*bodyInterface*/) {}

void ActorUpdate(Actor * /*this*/, double /*delta*/) {}

void ActorDestroy(Actor * /*this*/) {}

ActorInitFunction ActorInitFuncs[] = {
	ActorInit,
	TestActorInit,
	CoinInit,
	GoalInit,
	DoorInit,
	TriggerInit,
	IoProxyInit,
	PhysboxInit,
	LaserInit,
	StaticModelInit,
	SoundPlayerInit,
	SpriteInit,
	LaserEmitterInit,
	LogicBinaryInit,
	LogicDecimalInit,
	LogicCounterInit,
};

ActorUpdateFunction ActorUpdateFuncs[] = {
	ActorUpdate,
	TestActorUpdate,
	CoinUpdate,
	GoalUpdate,
	DoorUpdate,
	TriggerUpdate,
	IoProxyUpdate,
	PhysboxUpdate,
	LaserUpdate,
	ActorUpdate,
	ActorUpdate,
	ActorUpdate,
	LaserEmitterUpdate,
	ActorUpdate,
	ActorUpdate,
	ActorUpdate,
};

ActorDestroyFunction ActorDestroyFuncs[] = {
	ActorDestroy,
	TestActorDestroy,
	CoinDestroy,
	GoalDestroy,
	DoorDestroy,
	TriggerDestroy,
	IoProxyDestroy,
	PhysboxDestroy,
	LaserDestroy,
	ActorDestroy,
	SoundPlayerDestroy,
	SpriteDestroy,
	LaserDestroy,
	LogicBinaryDestroy,
	LogicDecimalDestroy,
	LogicCounterDestroy,
};

Actor *CreateActor(const Transform *transform, const uint actorType, KvList *params, JPH_BodyInterface *bodyInterface)
{
	Actor *actor = malloc(sizeof(Actor));
	CheckAlloc(actor);
	actor->actorWall = NULL;
	actor->transform = *transform;
	actor->health = 1;
	actor->actorModel = NULL;
	actor->currentSkinIndex = 0;
	actor->currentLod = 0;
	ListInit(actor->ioConnections, LIST_POINTER);
	actor->SignalHandler = DefaultSignalHandler;
	actor->Init = ActorInitFuncs[actorType];
	actor->Update = ActorUpdateFuncs[actorType];
	actor->Destroy = ActorDestroyFuncs[actorType];
	actor->Init(actor, params, bodyInterface); // kindly allow the Actor to initialize itself
	actor->actorType = actorType;
	ActorFireOutput(actor, ACTOR_SPAWN_OUTPUT, PARAM_NONE);
	if (params)
	{
		KvListDestroy(params);
	}
	return actor;
}

void FreeActor(Actor *actor)
{
	actor->Destroy(actor);
	for (int i = 0; i < actor->ioConnections.length; i++)
	{
		ActorConnection *connection = ListGetPointer(actor->ioConnections, i);
		DestroyActorConnection(connection);
	}
	ListFree(actor->ioConnections);
	free(actor);
	actor = NULL;
}

void ActorTriggerInput(const Actor *sender, const Actor *receiver, const byte signal, const Param *param)
{
	LogInfo("Triggering input %d on actor %p from actor %p\n", signal, receiver, sender);
	if (receiver->SignalHandler != NULL)
	{
		const bool handled = receiver->SignalHandler((Actor *)receiver, sender, signal, param);
		if (!handled)
		{
			LogWarning("Signal %d was sent to actor %p but was not handled!", signal, receiver);
		}
	} else
	{
		LogWarning("Actor %p does not have a signal handler!", receiver);
	}
}

void ActorFireOutput(const Actor *sender, const byte signal, const Param defaultParam)
{
	//LogInfo("Firing signal %d from actor %p with param \"%s\"\n", signal, sender, defaultParam);
	ListLock(sender->ioConnections);
	for (int i = 0; i < sender->ioConnections.length; i++)
	{
		const ActorConnection *connection = ListGetPointer(sender->ioConnections, i);
		if (connection->myOutput == signal)
		{
			List actors;
			GetActorsByName(connection->outActorName, GetState()->level, &actors);
			if (actors.length == 0)
			{
				LogWarning("Tried to fire signal to actor %s, but it was not found!", connection->outActorName);
				continue;
			}
			for (int j = 0; j < actors.length; j++)
			{
				Actor *actor = ListGetPointer(actors, j);
				if (actor->SignalHandler != NULL)
				{
					const Param *param = &defaultParam;
					if (connection->outParamOverride.type != PARAM_TYPE_NONE)
					{
						param = &connection->outParamOverride;
					}
					const bool handled = actor->SignalHandler(actor, sender, connection->targetInput, param);
					if (!handled)
					{
						LogWarning("Signal %d was sent to actor %p but was not handled!", signal, actor);
					}
				}
			}
			ListFree(actors);
		}
	}
	ListUnlock(sender->ioConnections);
}

void DestroyActorConnection(ActorConnection *connection)
{
	free(connection);
}

bool DefaultSignalHandler(Actor *this, const Actor * /*sender*/, const byte signal, const Param * /*param*/)
{
	if (signal == ACTOR_KILL_INPUT)
	{
		RemoveActor(this);
		return true;
	}
	return false;
}
