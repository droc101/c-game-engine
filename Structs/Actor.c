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
void ActorInit(Actor * /*this*/, const KvList * /*params*/, Transform * /*transform*/) {}

void ActorUpdate(Actor * /*this*/, double /*delta*/) {}

void ActorDestroy(Actor * /*this*/) {}

ActorInitFunction actorInitFunctions[] = {
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

ActorUpdateFunction actorUpdateFunctions[] = {
	ActorUpdate,
	TestActorUpdate,
	CoinUpdate,
	GoalUpdate,
	DoorUpdate,
	ActorUpdate,
	IoProxyUpdate,
	ActorUpdate,
	LaserUpdate,
	ActorUpdate,
	ActorUpdate,
	ActorUpdate,
	LaserEmitterUpdate,
	ActorUpdate,
	ActorUpdate,
	ActorUpdate,
};

ActorDestroyFunction actorDestroyFunctions[] = {
	ActorDestroy,
	ActorDestroy,
	ActorDestroy,
	ActorDestroy,
	DoorDestroy,
	ActorDestroy,
	ActorDestroy,
	ActorDestroy,
	ActorDestroy,
	ActorDestroy,
	ActorDestroy,
	ActorDestroy,
	ActorDestroy,
	ActorDestroy,
	ActorDestroy,
	ActorDestroy,
};

Actor *CreateActor(Transform *transform, const ActorType actorType, KvList *params, JPH_BodyInterface *bodyInterface)
{
	Actor *actor = malloc(sizeof(Actor));
	CheckAlloc(actor);
	actor->actorFlags = 0;
	actor->bodyInterface = bodyInterface;
	actor->bodyId = JPH_BodyId_InvalidBodyID;
	actor->actorWall = NULL;
	actor->health = 1;
	actor->actorType = actorType;
	actor->actorModel = NULL;
	actor->currentSkinIndex = 0;
	actor->currentLod = 0;
	ListInit(actor->ioConnections, LIST_POINTER);
	actor->SignalHandler = DefaultSignalHandler;
	actor->Init = actorInitFunctions[actorType];
	actor->Update = actorUpdateFunctions[actorType];
	actor->Destroy = actorDestroyFunctions[actorType];
	actor->OnPlayerContactAdded = NULL;
	actor->OnPlayerContactPersisted = NULL;
	actor->OnPlayerContactRemoved = NULL;
	actor->extraData = NULL;
	actor->Init(actor, params, transform); // kindly allow the Actor to initialize itself
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
	free(actor->actorWall);
	actor->actorWall = NULL;
	free(actor->extraData);
	actor->extraData = NULL;
	if (actor->bodyId != JPH_BodyId_InvalidBodyID && actor->bodyInterface != NULL)
	{
		JPH_BodyInterface_RemoveAndDestroyBody(actor->bodyInterface, actor->bodyId);
	}
	for (size_t i = 0; i < actor->ioConnections.length; i++)
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
	for (size_t i = 0; i < sender->ioConnections.length; i++)
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
			for (size_t j = 0; j < actors.length; j++)
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

void ActorWallBake(const Actor *this)
{
	const float dx = this->actorWall->b.x - this->actorWall->a.x;
	const float dy = this->actorWall->b.y - this->actorWall->a.y;
	this->actorWall->length = sqrtf(dx * dx + dy * dy);
	if (this->bodyId != JPH_BodyId_InvalidBodyID && this->bodyInterface != NULL)
	{
		JPH_Quat rotation = {};
		JPH_BodyInterface_GetRotation(this->bodyInterface, this->bodyId, &rotation);
		this->actorWall->angle = JPH_Quat_GetRotationAngle(&rotation, &Vector3_AxisY);
	} else
	{
		this->actorWall->angle = atan2f(dy, dx);
	}
}
