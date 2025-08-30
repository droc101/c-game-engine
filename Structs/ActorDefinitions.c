//
// Created by NBT22 on 8/29/25.
//

#include "ActorDefinitions.h"
#include <stddef.h>
#include "../Helpers/Core/Error.h"

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

ActorInitFunctionDict initFunctions;

void RegisterActors()
{
	ActorInitFunctionDict_init(initFunctions);

	ActorInitFunctionDict_set_at(initFunctions, "test_actor", TestActorInit);
	ActorInitFunctionDict_set_at(initFunctions, "coin", CoinInit);
	ActorInitFunctionDict_set_at(initFunctions, "goal", GoalInit);
	ActorInitFunctionDict_set_at(initFunctions, "door", DoorInit);
	ActorInitFunctionDict_set_at(initFunctions, "trigger", TriggerInit);
	ActorInitFunctionDict_set_at(initFunctions, "io_proxy", IoProxyInit);
	ActorInitFunctionDict_set_at(initFunctions, "physbox", PhysboxInit);
	ActorInitFunctionDict_set_at(initFunctions, "laser", LaserInit);
	ActorInitFunctionDict_set_at(initFunctions, "static_model", StaticModelInit);
	ActorInitFunctionDict_set_at(initFunctions, "sound_player", SoundPlayerInit);
	ActorInitFunctionDict_set_at(initFunctions, "sprite", SpriteInit);
	ActorInitFunctionDict_set_at(initFunctions, "laser_emitter", LaserEmitterInit);
	ActorInitFunctionDict_set_at(initFunctions, "logic_binary", LogicBinaryInit);
	ActorInitFunctionDict_set_at(initFunctions, "logic_decimal", LogicDecimalInit);
	ActorInitFunctionDict_set_at(initFunctions, "logic_counter", LogicCounterInit);
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
