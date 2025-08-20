//
// Created by droc101 on 4/22/2024.
//

#include "Actor.h"
#include <assert.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/List.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/Physics/Physics.h"
#include "Color.h"
#include "GlobalState.h"
#include "Level.h"

#include <joltc/constants.h>
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

void ActorRender(Actor * /*this*/) {}

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
	actor->modColor = COLOR_WHITE;
	ListInit(actor->ioConnections, LIST_POINTER);
	actor->Init = actorInitFunctions[actorType];
	actor->Update = ActorUpdate;
	actor->Destroy = ActorDestroy;
	actor->SignalHandler = DefaultSignalHandler;
	actor->Render = ActorRender;
	actor->OnPlayerContactAdded = NULL;
	actor->OnPlayerContactPersisted = NULL;
	actor->OnPlayerContactRemoved = NULL;
	actor->extraData = NULL;
	actor->Init(actor, params, transform); // kindly allow the Actor to initialize itself
	ActorFireOutput(actor, ACTOR_OUTPUT_SPAWNED, PARAM_NONE);
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

void ActorTriggerInput(const Actor *sender, const Actor *receiver, const uint8_t signal, const Param *param)
{
	LogInfo("Triggering input %d on actor %p from actor %p\n", signal, receiver, sender);
	assert(receiver->SignalHandler != NULL);
	if (!receiver->SignalHandler((Actor *)receiver, sender, signal, param))
	{
		LogWarning("Signal %d was sent to actor %p but was not handled!", signal, receiver);
	}
}

void ActorFireOutput(const Actor *sender, const uint8_t signal, const Param defaultParam)
{
	// LogInfo("Firing signal %d from actor %p with param \"%s\"\n", signal, sender, defaultParam);
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
				assert(actor->SignalHandler != NULL);
				const Param *param = &defaultParam;
				if (connection->outParamOverride.type != PARAM_TYPE_NONE)
				{
					param = &connection->outParamOverride;
				}
				if (!actor->SignalHandler(actor, sender, connection->targetInput, param))
				{
					LogWarning("Signal %d was sent to actor %p but was not handled!", signal, actor);
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

bool DefaultSignalHandler(Actor *this, const Actor * /*sender*/, const uint8_t signal, const Param * /*param*/)
{
	if (signal == ACTOR_INPUT_KILL)
	{
		RemoveActor(this);
		return true;
	}
	return false;
}

void ActorCreateEmptyBody(Actor *this, const Transform *transform)
{
	const JPH_ShapeSettings *shapeSettings = (JPH_ShapeSettings *)JPH_EmptyShapeSettings_Create(&Vector3_Zero);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create_GAME(shapeSettings,
																						  transform,
																						  JPH_MotionType_Static,
																						  OBJECT_LAYER_STATIC,
																						  this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_DontActivate);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}
