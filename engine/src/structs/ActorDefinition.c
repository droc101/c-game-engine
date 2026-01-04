//
// Created by NBT22 on 8/29/25.
//

#include <assert.h>
#include <engine/actor/GlobalLight.h>
#include <engine/actor/IoProxy.h>
#include <engine/actor/LogicBinary.h>
#include <engine/actor/LogicCounter.h>
#include <engine/actor/LogicDecimal.h>
#include <engine/actor/SoundPlayer.h>
#include <engine/actor/Sprite.h>
#include <engine/actor/StaticModel.h>
#include <engine/actor/Trigger.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <stddef.h>

static ActorDefinitionDict actorDefinitions;

void RegisterActor(const char *actorTypeName, ActorDefinition *definition)
{
	// Please don't be bad.
	assert(ActorDefinitionDict_get(actorDefinitions, actorTypeName) == NULL); // Actor name already registered
	assert(actorTypeName != NULL);
	assert(definition != NULL);
	assert(definition->Update != NULL);
	assert(definition->OnPlayerContactAdded != NULL);
	assert(definition->OnPlayerContactPersisted != NULL);
	assert(definition->OnPlayerContactRemoved != NULL);
	assert(definition->RenderUi != NULL);
	assert(definition->Init != NULL);
	assert(definition->Destroy != NULL);
#ifdef BUILDSTYLE_DEBUG
	ActorInputHandlerFunctionDict_iterator it;
	ActorInputHandlerFunctionDict_it(it, definition->inputHandlers);
	while (!ActorInputHandlerFunctionDict_last_p(it))
	{
		const ActorInputHandlerFunctionDict_pair *pair = ActorInputHandlerFunctionDict_ref(it);
		assert(pair->value != NULL);
		ActorInputHandlerFunctionDict_next(it);
	}
#endif
	ActorDefinitionDict_set_at(actorDefinitions, actorTypeName, definition);
}

void RegisterActorInput(ActorDefinition *definition, const char *name, const ActorInputHandlerFunction handler)
{
	assert(definition != NULL);
	assert(handler != NULL);
	assert(name != NULL);
	assert(ActorInputHandlerFunctionDict_get(definition->inputHandlers, name) ==
		   NULL); // Signal name already registered
	ActorInputHandlerFunctionDict_set_at(definition->inputHandlers, name, handler);
}

void UnregisterActorInput(ActorDefinition *definition, const char *name)
{
	assert(definition != NULL);
	assert(name != NULL);
	ActorInputHandlerFunctionDict_erase(definition->inputHandlers, name);
}

void RegisterDefaultActorInputs(ActorDefinition *definition)
{
	ActorInputHandlerFunctionDict_init(definition->inputHandlers);
	RegisterActorInput(definition, ACTOR_INPUT_KILL, ActorSignalKill);
}

void RegisterActors(const RegisterGameActorsFunction RegisterGameActors)
{
	LogDebug("Registering actors...\n");
	ActorDefinitionDict_init(actorDefinitions);

	RegisterIoProxy();
	RegisterLogicBinary();
	RegisterLogicCounter();
	RegisterLogicDecimal();
	RegisterSoundPlayer();
	RegisterSprite();
	RegisterStaticModel();
	RegisterTrigger();
	RegisterGlobalLight();
	RegisterGameActors();
}

const ActorDefinition *GetActorDefinition(const char *actorType)
{
	ActorDefinition **definition = ActorDefinitionDict_get(actorDefinitions, actorType);
	if (definition == NULL)
	{
		Error("Unknown actor type!");
	}
	return *definition;
}

ActorInputHandlerFunction GetActorInputHandler(const ActorDefinition *definition, const char *input)
{
	const ActorInputHandlerFunction *handler = ActorInputHandlerFunctionDict_get(definition->inputHandlers, input);
	if (handler == NULL)
	{
		return NULL;
	}
	return *handler;
}

void DestroyActorDefinitions()
{
	ActorDefinitionDict_iterator it;
	ActorDefinitionDict_it(it, actorDefinitions);
	while (!ActorDefinitionDict_last_p(it))
	{
		const ActorDefinitionDict_pair *pair = ActorDefinitionDict_ref(it);
		ActorInputHandlerFunctionDict_clear(pair->value->inputHandlers);
		ActorDefinitionDict_next(it);
	}

	ActorDefinitionDict_clear(actorDefinitions);
}
