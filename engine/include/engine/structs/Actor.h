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
#include <engine/assets/ModelLoader.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Color.h>
#include <engine/structs/Param.h>
#include <engine/structs/Wall.h>

typedef struct Actor Actor;
typedef struct ActorConnection ActorConnection;

#define ACTOR_INPUT_KILL "kill"

#define ACTOR_OUTPUT_SPAWNED "spawned"
#define ACTOR_OUTPUT_KILLED "killed"

enum ActorFlags
{
	ACTOR_FLAG_CAN_PUSH_PLAYER = 1 << 0,
	ACTOR_FLAG_ENEMY = 1 << 1,
	ACTOR_FLAG_CAN_BLOCK_LASERS = 1 << 2, // TODO should we split this enum into engine-side and game-side
	ACTOR_FLAG_CAN_BE_HELD = 1 << 3,
	ACTOR_FLAG_USING_BOUNDING_BOX_COLLISION = 1 << 4,
};

struct ActorConnection
{
	/// The name of the input on the target actor
	char *targetActorInput;
	/// The name of the output on the source actor
	char *sourceActorOutput;
	/// The name of the actor firing the output
	char *targetActorName;
	/// The param to send to the input
	Param outParamOverride;
	size_t numRefires;
};

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

	/// The actor's definition
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
Actor *CreateActor(Transform *transform, const char *actorType, KvList params, JPH_BodyInterface *bodyInterface);

/**
 * Destroy an Actor
 * @param actor actor to destroy
 */
void FreeActor(Actor *actor);

/**
 * Directly trigger an input on an actor
 * @param sender The actor sending the signal
 * @param receiver The actor receiving the signal
 * @param input The signal to send
 * @param param The parameter to send with the signal
 */
void ActorTriggerInput(const Actor *sender, Actor *receiver, const char *input, const Param *param);

/**
 * Fire signal from an actor
 * @param sender The actor sending the signal
 * @param output The signal to send
 * @param defaultParam The default parameter to send with the signal
 */
void ActorFireOutput(const Actor *sender, const char *output, Param defaultParam);

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

/**
 * Default actor update function
 */
void DefaultActorUpdate(Actor * /*this*/, double /*delta*/);

/**
 * Actor kill signal handler
 * @param this The actor this signal was sent to
 */
void ActorSignalKill(Actor *this, const Actor * /*sender*/, const Param * /*param*/);

/**
 * Default actor on player contact added function
 */
void DefaultActorOnPlayerContactAdded(Actor * /*this*/, JPH_BodyId /*bodyId*/);

/**
 * Default actor on player contact persisted function
 */
void DefaultActorOnPlayerContactPersisted(Actor * /*this*/, JPH_BodyId /*bodyId*/);

/**
 * Default actor on player contact removed function
 */
void DefaultActorOnPlayerContactRemoved(Actor * /*this*/, JPH_BodyId /*bodyId*/);

/**
 * Default actor render ui function
 */
void DefaultActorRenderUi(Actor * /*this*/);

/**
 * Default actor destroy function
 */
void DefaultActorDestroy(Actor * /*this*/);


#endif //GAME_ACTOR_H
