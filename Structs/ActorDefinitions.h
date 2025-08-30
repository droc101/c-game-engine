//
// Created by NBT22 on 8/29/25.
//

#ifndef GAME_ACTORDEFINITIONS_H
#define GAME_ACTORDEFINITIONS_H

#include <joltc/Math/Transform.h>
#include <joltc/types.h>
#include <stdbool.h>
#include <stdint.h>
#include "../Helpers/Core/Dict.h"
#include "../Helpers/Core/KVList.h"

typedef struct Actor Actor;


typedef void (*ActorInitFunction)(Actor *this, const KvList *params, Transform *transform);

typedef void (*ActorUpdateFunction)(Actor *this, double delta);

typedef void (*ActorDestroyFunction)(Actor *this);

typedef void (*ActorUIRenderFunction)(Actor *this);

/**
 * Signal handler function signature for actor
 * @return True if the signal was handled, false if not
 */
typedef bool (*ActorSignalHandlerFunction)(Actor *this, const Actor *sender, uint8_t signal, const Param *param);

typedef void (*ActorPlayerContactAddedFunction)(Actor *this, JPH_BodyId bodyId);

typedef void (*ActorPlayerContactPersistedFunction)(Actor *this, JPH_BodyId bodyId);

typedef void (*ActorPlayerContactRemovedFunction)(Actor *this, JPH_BodyId bodyId);


typedef enum ActorType ActorType;

typedef struct ActorDefinition ActorDefinition;


enum ActorType
{
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

struct ActorDefinition
{
	/// The actor type index
	/// @warning Do not change this after creation
	ActorType actorType;
	/// The function to call when the actor is updated
	/// @note This should be called every tick
	ActorUpdateFunction Update;
	/// The function to call when the actor receives a signal.
	ActorSignalHandlerFunction SignalHandler;
	ActorPlayerContactAddedFunction OnPlayerContactAdded;
	ActorPlayerContactPersistedFunction OnPlayerContactPersisted;
	ActorPlayerContactRemovedFunction OnPlayerContactRemoved;
	ActorUIRenderFunction RenderUi;

	/// The function to call when the actor is destroyed
	/// @note This should only be called once, when the actor is destroyed
	ActorDestroyFunction Destroy;
};

DEFINE_DICT(ActorInitFunctionDict, const char *, M_CSTR_OPLIST, ActorInitFunction, M_PTR_OPLIST);

void RegisterActors();

ActorInitFunction GetActorInitFunction(const char *actorType);

void DestroyActorInitFunctionDictionary();

#endif //GAME_ACTORDEFINITIONS_H
