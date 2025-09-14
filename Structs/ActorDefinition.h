//
// Created by NBT22 on 8/29/25.
//

#ifndef GAME_ACTORDEFINITIONS_H
#define GAME_ACTORDEFINITIONS_H

#include <joltc/Math/Transform.h>
#include <joltc/types.h>
#include <m-core.h>
#include "../Helpers/Core/Dict.h"
#include "../Helpers/Core/KVList.h"
#include "Param.h"

typedef struct Actor Actor;

typedef enum ActorType ActorType;

typedef struct ActorDefinition ActorDefinition;

typedef void (*ActorInitFunction)(Actor *this, const KvList params, Transform *transform);

typedef void (*ActorUpdateFunction)(Actor *this, double delta);

typedef void (*ActorDestroyFunction)(Actor *this);

typedef void (*ActorUIRenderFunction)(Actor *this);

typedef void (*ActorInputHandlerFunction)(Actor *this, const Actor *sender, const Param *param);

typedef void (*ActorPlayerContactAddedFunction)(Actor *this, JPH_BodyId bodyId);

typedef void (*ActorPlayerContactPersistedFunction)(Actor *this, JPH_BodyId bodyId);

typedef void (*ActorPlayerContactRemovedFunction)(Actor *this, JPH_BodyId bodyId);

typedef void (*ActorRegisterFunction)();

DEFINE_DICT(ActorInputHandlerFunctionDict, const char *, M_CSTR_OPLIST, ActorInputHandlerFunction, M_PTR_OPLIST);

DEFINE_DICT(ActorDefinitionDict, const char *, M_CSTR_OPLIST, ActorDefinition *, M_PTR_OPLIST);

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

	ActorPlayerContactAddedFunction OnPlayerContactAdded;
	ActorPlayerContactPersistedFunction OnPlayerContactPersisted;
	ActorPlayerContactRemovedFunction OnPlayerContactRemoved;

	/// The function called to allow the actor to render UI
	ActorUIRenderFunction RenderUi;

	/// The list of input handlers
	ActorInputHandlerFunctionDict inputHandlers;


	ActorInitFunction Init;
	/// The function to call when the actor is destroyed
	/// @note This should only be called once, when the actor is destroyed
	ActorDestroyFunction Destroy;
};

/**
 * Register an actor type
 * @param actorTypeName The name of the actor type
 * @param definition The actor's definition
 */
void RegisterActor(const char *actorTypeName, ActorDefinition *definition);

void RegisterActorInput(ActorDefinition *definition, const char *name, ActorInputHandlerFunction handler);

void UnregisterActorInput(ActorDefinition *definition, const char *name);

void RegisterDefaultActorInputs(ActorDefinition *definition);

/**
 * Register all actor types
 */
void RegisterActors();

const ActorDefinition *GetActorDefinition(const char *actorType);

ActorInputHandlerFunction GetActorInputHandler(const ActorDefinition *definition, const char *input);

/**
 * Destroy actor registrations
 */
void DestroyActorDefinitions();

#endif //GAME_ACTORDEFINITIONS_H
