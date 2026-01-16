//
// Created by NBT22 on 8/29/25.
//

#ifndef GAME_ACTORDEFINITIONS_H
#define GAME_ACTORDEFINITIONS_H

#include <engine/structs/Dict.h>
#include <engine/structs/KVList.h>
#include <joltc/Math/Transform.h>
#include <joltc/types.h>
#include <m-core.h>
#include <stdint.h>

#define FIRST_GAME_ACTOR_TYPE_ID (1 << 16)

typedef struct Actor Actor;

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

typedef void (*RegisterGameActorsFunction)();

DEFINE_DICT(ActorInputHandlerFunctionDict, const char *, M_CSTR_OPLIST, ActorInputHandlerFunction, M_PTR_OPLIST);

DEFINE_DICT(ActorDefinitionDict, const char *, M_CSTR_OPLIST, ActorDefinition *, M_PTR_OPLIST);

struct ActorDefinition
{
	/// The actor type index
	/// @warning Do not change this after creation
	uint64_t actorType;
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

	/// The function to call to initialize the actor
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

/**
 * Register an input handler to an actor definition
 * @param definition The actor definition to modify
 * @param name The name of the input
 * @param handler The handler function
 */
void RegisterActorInput(ActorDefinition *definition, const char *name, ActorInputHandlerFunction handler);

/**
 * Remove an input handler from an actor definition
 * @param definition The actor definition to modify
 * @param name The input name to remove
 */
void UnregisterActorInput(ActorDefinition *definition, const char *name);

/**
 * Register default actor inputs
 * @param definition The definition to modify
 */
void RegisterDefaultActorInputs(ActorDefinition *definition);

/**
 * Register all actor types
 */
void RegisterActors(RegisterGameActorsFunction RegisterGameActors);

/**
 * Get the actor definition for an actor type name
 * @param actorType The actor type name
 * @return The actor's definition
 */
const ActorDefinition *GetActorDefinition(const char *actorType);

/**
 * Get an input handler for an actor definition
 * @param definition The actor definition
 * @param input The signal name
 * @return The signal handler function
 */
ActorInputHandlerFunction GetActorInputHandler(const ActorDefinition *definition, const char *input);

/**
 * Destroy actor registrations
 */
void DestroyActorDefinitions();

#endif //GAME_ACTORDEFINITIONS_H
