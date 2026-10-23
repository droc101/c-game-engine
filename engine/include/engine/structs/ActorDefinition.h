//
// Created by NBT22 on 8/29/25.
//

/**
 * @file ActorDefinition.h
 * Actor class registration system
 */

#ifndef GAME_ACTORDEFINITIONS_H
#define GAME_ACTORDEFINITIONS_H

#include <engine/structs/Dict.h>
#include <engine/structs/KVList.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyID.h>
#include <m-core.h>

typedef struct Actor Actor;

typedef struct ActorDefinition ActorDefinition;

/**
 * Actor initialization function signature
 * @param this The actor to initialize
 * @param params The params to initialize with
 * @param transform The transform to initialize with
 */
typedef void (*ActorInitFunction)(Actor *this, const KvList params, const Transform *transform);

/**
 * Actor update function signature
 * @param this The actor to update
 * @param delta Physics delta
 */
typedef void (*ActorUpdateFunction)(Actor *this, double delta);

/**
 * Actor destroy/free function signature
 * @param this The actor to destroy
 */
typedef void (*ActorDestroyFunction)(Actor *this);

/**
 * Actor UI rendering function
 * @param this The actor to render UI for
 */
typedef void (*ActorUIRenderFunction)(Actor *this);

/**
 * Actor input/signal handler function
 * @param this The actor that received the signal
 * @param sender The actor that sent this signal
 * @param param The @c Param send with this signal
 */
typedef void (*ActorInputHandlerFunction)(Actor *this, const Actor *sender, const Param *param);

/**
 * Actor player contact added handler function
 * @param this The actor to handle contact for
 * @param bodyId The player
 */
typedef void (*ActorPlayerContactAddedFunction)(Actor *this, JPH_BodyID bodyId);

/**
 * Actor player contact persisted handler function
 * @param this The actor to handle contact for
 * @param bodyId The player
 */
typedef void (*ActorPlayerContactPersistedFunction)(Actor *this, JPH_BodyID bodyId);

/**
 * Actor player contact removed handler function
 * @param this The actor to handle contact for
 * @param bodyId The player
 */
typedef void (*ActorPlayerContactRemovedFunction)(Actor *this, JPH_BodyID bodyId);

/**
 * Actor interaction function signature
 * @param this The actor the player interacted with
 */
typedef void (*ActorInteractFunction)(Actor *this);

/// Function signature used to register an actor
typedef void (*ActorRegisterFunction)(void);

/// Function signature used for game-side actor registration
typedef void (*RegisterGameActorsFunction)(void);

/// @private
DEFINE_DICT(ActorInputHandlerFunctionDict, const char *, M_CSTR_OPLIST, ActorInputHandlerFunction, M_PTR_OPLIST);
/// @private
DEFINE_DICT(ActorDefinitionDict, const char *, M_CSTR_OPLIST, ActorDefinition *, M_PTR_OPLIST);

/// Actor class definition
struct ActorDefinition
{
	/// The function to call when the actor is updated
	/// @note This should be called every tick
	ActorUpdateFunction Update;

	/// The function called when the player begins to contact this actor
	ActorPlayerContactAddedFunction OnPlayerContactAdded;
	/// The function called every tick the player is contacting this actor
	ActorPlayerContactPersistedFunction OnPlayerContactPersisted;
	/// The function called when the player stops contacting this actor
	ActorPlayerContactRemovedFunction OnPlayerContactRemoved;

	/// The function called to allow the actor to render UI
	ActorUIRenderFunction RenderUi;

	/// The list of input handlers
	ActorInputHandlerFunctionDict inputHandlers;

	/// The function to call when the interact key is pressed on this actor, if it has the @c ACTOR_FLAG_INTERACTABLE flag
	ActorInteractFunction Interact;

	/// The function to call to initialize the actor
	ActorInitFunction Init;
	/// The function to call when the actor is destroyed
	/// @note This should only be called once, when the actor is destroyed
	ActorDestroyFunction Destroy;

	/// The class name of this actor definition
	const char *className;
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
 * @private
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
 * @private
 */
void DestroyActorDefinitions();

#endif //GAME_ACTORDEFINITIONS_H
