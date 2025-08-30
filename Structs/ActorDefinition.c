//
// Created by NBT22 on 8/29/25.
//

#include "ActorDefinition.h"
#include <assert.h>
#include <stddef.h>
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
#include "../Helpers/Core/Error.h"

ActorInitFunctionDict initFunctions;

void RegisterActor(const char *actorTypeName, const ActorInitFunction actorInitFunction)
{
	assert(ActorInitFunctionDict_get(initFunctions, actorTypeName) == NULL); // Actor name already registered
	assert(actorInitFunction != NULL); // Bad.
	ActorInitFunctionDict_set_at(initFunctions, actorTypeName, actorInitFunction);
}

void RegisterActors()
{
	ActorInitFunctionDict_init(initFunctions);

	// prop_*
	RegisterActor(COIN_ACTOR_NAME, CoinInit);
	RegisterActor(GOAL_ACTOR_NAME, GoalInit);
	RegisterActor(DOOR_ACTOR_NAME, DoorInit);
	RegisterActor(PHYSBOX_ACTOR_NAME, PhysboxInit);
	RegisterActor(LASER_ACTOR_NAME, LaserInit);
	RegisterActor(STATIC_MODEL_ACTOR_NAME, StaticModelInit);
	RegisterActor(SOUND_PLAYER_ACTOR_NAME, SoundPlayerInit);
	RegisterActor(SPRITE_ACTOR_NAME, SpriteInit);
	RegisterActor(LASER_EMITTER_ACTOR_NAME, LaserEmitterInit);

	// logic_*
	RegisterActor(LOGIC_BINARY_ACTOR_NAME, LogicBinaryInit);
	RegisterActor(LOGIC_DECIMAL_ACTOR_NAME, LogicDecimalInit);
	RegisterActor(LOGIC_COUNTER_ACTOR_NAME, LogicCounterInit);

	// misc
	RegisterActor(TEST_ACTOR_NAME, TestActorInit);
	RegisterActor(TRIGGER_ACTOR_NAME, TriggerInit);
	RegisterActor(IO_PROXY_ACTOR_NAME, IoProxyInit);
}

ActorInitFunction GetActorInitFunction(const char *actorType)
{
	const ActorInitFunction *initFunctionPtr = ActorInitFunctionDict_get(initFunctions, actorType);
	if (initFunctionPtr == NULL)
	{
		Error("Unknown actor type!");
	}
	return *initFunctionPtr;
}

void DestroyActorInitFunctionDictionary()
{
	ActorInitFunctionDict_clear(initFunctions);
}
