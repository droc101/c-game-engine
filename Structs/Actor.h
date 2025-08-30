//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_ACTOR_H
#define GAME_ACTOR_H

#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <joltc/types.h>
#include <stdbool.h>
#include <stdint.h>
#include "../Helpers/Core/AssetLoaders/ModelLoader.h"
#include "../Helpers/Core/KVList.h"
#include "../Helpers/Core/List.h"
#include "ActorDefinition.h"
#include "Color.h"
#include "Wall.h"

typedef struct Actor Actor;
typedef struct ActorConnection ActorConnection;

enum ActorInput
{
	ACTOR_INPUT_KILL = 0,
};

enum ActorOutput
{
	ACTOR_OUTPUT_SPAWNED = 1,
	ACTOR_OUTPUT_KILLED = 0,
};

enum ActorFlags
{
	ACTOR_FLAG_CAN_PUSH_PLAYER = 1 << 0,
	ACTOR_FLAG_ENEMY = 1 << 1,
	ACTOR_FLAG_CAN_BLOCK_LASERS = 1 << 2,
	ACTOR_FLAG_CAN_BE_HELD = 1 << 3,
};

struct ActorConnection
{
	uint8_t targetInput;
	uint8_t myOutput;
	char outActorName[64];
	Param outParamOverride;
};

// Actor (interactable/moving wall) struct
struct Actor
{
	/// Flags used to provide more information about the actor
	uint32_t actorFlags;

	JPH_BodyId bodyId;
	JPH_BodyInterface *bodyInterface;

	/// Optional model for the actor, if not NULL, will be rendered instead of the wall
	ModelDefinition *actorModel;
	/// The index of the active skin for the actor's model
	uint32_t currentSkinIndex;
	/// The current LOD level of the actor's model, re-calculated each physics tick
	uint32_t currentLod;
	/// The color modifier of the actor's model
	Color modColor;

	/// The actor's wall, in global space
	ActorWall *actorWall;

	const ActorDefinition *definition;

	/// List of I/O connections
	LockingList ioConnections;

	/// The actor's health
	/// @note May be unused for some actors
	int health;
	/// Extra data for the actor
	void *extraData;
};

/**
 * Create an Actor
 * @param transform Actor position
 * @param actorType Actor type
 * @param params Parameters for the actor, can be NULL
 * @param bodyInterface The Joly body interface within which to create the actor's rigid body
 * @return Initialized Actor struct
 */
Actor *CreateActor(Transform *transform, const char *actorType, KvList *params, JPH_BodyInterface *bodyInterface);

/**
 * Destroy an Actor
 * @param actor actor to destroy
 */
void FreeActor(Actor *actor);

/**
 * Directly trigger an input on an actor
 * @param sender The actor sending the signal
 * @param receiver The actor receiving the signal
 * @param signal The signal to send
 * @param param The parameter to send with the signal
 */
void ActorTriggerInput(const Actor *sender, const Actor *receiver, uint8_t signal, const Param *param);

/**
 * Fire signal from an actor
 * @param sender The actor sending the signal
 * @param signal The signal to send
 * @param defaultParam The default parameter to send with the signal
 */
void ActorFireOutput(const Actor *sender, uint8_t signal, Param defaultParam);

/**
 * Destroy an actor connection
 * @param connection The connection to destroy
 */
void DestroyActorConnection(ActorConnection *connection);

/**
 * Create an empty body for an actor which does not need collision, but does need a position in the world
 * @param this The actor to create the body for
 * @param transform The transform with which to create the body
 */
void ActorCreateEmptyBody(Actor *this, const Transform *transform);


void DefaultActorUpdate(Actor * /*this*/, double /*delta*/);

/**
 * Default signal handler for actors, handling global signals such as kill
 * @return Whether the signal was handled
 */
bool DefaultActorSignalHandler(Actor *this, const Actor *sender, uint8_t signal, const Param *param);

void DefaultActorOnPlayerContactAdded(Actor * /*this*/, JPH_BodyId /*bodyId*/);

void DefaultActorOnPlayerContactPersisted(Actor * /*this*/, JPH_BodyId /*bodyId*/);

void DefaultActorOnPlayerContactRemoved(Actor * /*this*/, JPH_BodyId /*bodyId*/);

void DefaultActorRenderUi(Actor * /*this*/);

void DefaultActorDestroy(Actor * /*this*/);


#endif //GAME_ACTOR_H
