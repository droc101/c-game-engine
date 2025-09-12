//
// Created by droc101 on 4/22/2024.
//

#include "Actor.h"
#include <assert.h>
#include <joltc/constants.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <joltc/Physics/Body/BodyCreationSettings.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/Physics/Collision/Shape/Shape.h>
#include <joltc/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/List.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/Physics/Physics.h"
#include "ActorDefinition.h"
#include "Color.h"
#include "GlobalState.h"
#include "Level.h"

Actor *CreateActor(Transform *transform, const char *actorType, KvList params, JPH_BodyInterface *bodyInterface)
{
	Actor *actor = malloc(sizeof(Actor));
	CheckAlloc(actor);
	actor->actorFlags = 0;
	actor->bodyInterface = bodyInterface;
	actor->bodyId = JPH_BodyId_InvalidBodyID;
	actor->actorWall = NULL;
	actor->health = 1;
	actor->actorModel = NULL;
	actor->currentSkinIndex = 0;
	actor->currentLod = 0;
	actor->modColor = COLOR_WHITE;
	ListInit(actor->ioConnections, LIST_POINTER);
	actor->extraData = NULL;
	const ActorInitFunction initFunction = GetActorInitFunction(actorType);
	initFunction(actor, params, transform); // kindly allow the Actor to initialize itself
	ActorFireOutput(actor, ACTOR_OUTPUT_SPAWNED, PARAM_NONE);
	if (params)
	{
		KvListDestroy(params);
	}
	return actor;
}

void FreeActor(Actor *actor)
{
	actor->definition->Destroy(actor);
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
	if (!receiver->definition->SignalHandler((Actor *)receiver, sender, signal, param))
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
				const Param *param = &defaultParam;
				if (connection->outParamOverride.type != PARAM_TYPE_NONE)
				{
					param = &connection->outParamOverride;
				}
				if (!actor->definition->SignalHandler(actor, sender, connection->targetInput, param))
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
void DefaultActorUpdate(Actor * /*this*/, double /*delta*/) {}

bool DefaultActorSignalHandler(Actor *this, const Actor * /*sender*/, const uint8_t signal, const Param * /*param*/)
{
	if (signal == ACTOR_INPUT_KILL)
	{
		RemoveActor(this);
		return true;
	}
	return false;
}

void DefaultActorOnPlayerContactAdded(Actor * /*this*/, JPH_BodyId /*bodyId*/) {}

void DefaultActorOnPlayerContactPersisted(Actor * /*this*/, JPH_BodyId /*bodyId*/) {}

void DefaultActorOnPlayerContactRemoved(Actor * /*this*/, JPH_BodyId /*bodyId*/) {}

void DefaultActorRenderUi(Actor * /*this*/) {}

void DefaultActorDestroy(Actor * /*this*/) {}

void ActorCreateEmptyBody(Actor *this, const Transform *transform)
{
	JPH_ShapeSettings *shapeSettings = (JPH_ShapeSettings *)JPH_EmptyShapeSettings_Create(&Vector3_Zero);
	JPH_BodyCreationSettings *bodyCreationSettings = JPH_BodyCreationSettings_Create_GAME(shapeSettings,
																						  transform,
																						  JPH_MotionType_Static,
																						  OBJECT_LAYER_STATIC,
																						  this);
	this->bodyId = JPH_BodyInterface_CreateAndAddBody(this->bodyInterface,
													  bodyCreationSettings,
													  JPH_Activation_DontActivate);
	JPH_ShapeSettings_Destroy(shapeSettings);
	JPH_BodyCreationSettings_Destroy(bodyCreationSettings);
}
