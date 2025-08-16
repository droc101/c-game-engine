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
#include "Color.h"
#include "Wall.h"

enum ActorInput
{
	ACTOR_INPUT_KILL = 0,
};

enum ActorOutput
{
	ACTOR_OUTPUT_SPAWNED = 1,
	ACTOR_OUTPUT_KILLED = 0,
};

typedef enum ActorType ActorType;

typedef struct Actor Actor;
typedef struct ActorConnection ActorConnection;

typedef void (*ActorInitFunction)(Actor *this, const KvList *params, Transform *transform);

typedef void (*ActorUpdateFunction)(Actor *this, double delta);

typedef void (*ActorDestroyFunction)(Actor *this);

/**
 * Signal handler function signature for actor
 * @return True if the signal was handled, false if not
 */
typedef bool (*ActorSignalHandlerFunction)(Actor *this, const Actor *sender, uint8_t signal, const Param *param);

typedef void (*ActorPlayerContactAddedFunction)(Actor *this, JPH_BodyId bodyId);

typedef void (*ActorPlayerContactPersistedFunction)(Actor *this, JPH_BodyId bodyId);

typedef void (*ActorPlayerContactRemovedFunction)(Actor *this, JPH_BodyId bodyId);


enum ActorFlags
{
	ACTOR_FLAG_ENEMY = 1 << 0,
	ACTOR_FLAG_CAN_BLOCK_LASERS = 1 << 1,
	ACTOR_FLAG_CAN_BE_HELD = 1 << 2,
};

enum ActorType
{
	ACTOR_TYPE_EMPTY,
	ACTOR_TYPE_TEST,
	ACTOR_TYPE_COIN,
	ACTOR_TYPE_GOAL,
	ACTOR_TYPE_DOOR,
	ACTOR_TYPE_TRIGGER,
	ACTOR_TYPE_IO_PROXY,
	ACTOR_TYPE_PHYSBOX,
	ACTOR_TYPE_LASER,
	ACTOR_TYPE_STATIC_MODEL,
	ACTOR_TYPE_SOUND_PLAYER,
	ACTOR_TYPE_SPRITE,
	ACTOR_TYPE_LASER_EMITTER,
	ACTOR_TYPE_LOGIC_BINARY,
	ACTOR_TYPE_LOGIC_DECIMAL,
	ACTOR_TYPE_LOGIC_COUNTER
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

	JPH_BodyInterface *bodyInterface;
	JPH_BodyId bodyId;

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

	/// The actor type index
	/// @warning Do not change this after creation
	ActorType actorType;
	/// The function to call when the actor is initialized
	/// @note This should only be called once, when the actor is created
	ActorInitFunction Init;
	/// The function to call when the actor is updated
	/// @note This should be called every tick
	ActorUpdateFunction Update;
	/// The function to call when the actor is destroyed
	/// @note This should only be called once, when the actor is destroyed
	ActorDestroyFunction Destroy;
	/// The function to call when the actor receives a signal.
	ActorSignalHandlerFunction SignalHandler;
	ActorPlayerContactAddedFunction OnPlayerContactAdded;
	ActorPlayerContactPersistedFunction OnPlayerContactPersisted;
	ActorPlayerContactRemovedFunction OnPlayerContactRemoved;
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
Actor *CreateActor(Transform *transform, ActorType actorType, KvList *params, JPH_BodyInterface *bodyInterface);

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
 * Default signal handler for actors, handling global signals such as kill
 * @return Whether the signal was handled
 */
bool DefaultSignalHandler(Actor *this, const Actor *sender, uint8_t signal, const Param *param);

/**
 * Create an empty body for an actor which does not need collision, but does need a position in the world
 * @param this The actor to create the body for
 * @param transform The transform with which to create the body
 */
void ActorCreateEmptyBody(Actor *this, const Transform *transform);

#endif //GAME_ACTOR_H
