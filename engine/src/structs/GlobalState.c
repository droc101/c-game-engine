//
// Created by droc101 on 4/22/2024.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/MapLoader.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/Arguments.h>
#include <engine/physics/Physics.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Item.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Options.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/threads/PhysicsThread.h>
#include <SDL3/SDL_mouse.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HEALTH 100
#define MAX_MAP_PATH_LENGTH 80

static GlobalState state;

static const GameState *queuedStateChange = NULL;

void InitOptions()
{
	LogDebug("Loading options...\n");
	LoadOptions(&state.options);
}

void InitState()
{
	LogDebug("Initializing global state...\n");
	state.saveData = calloc(1, sizeof(SaveData));
	CheckAlloc(state.saveData);
	state.saveData->hp = 100;
	state.camera = NULL;
	state.rpcState = IN_MENUS;
	ListInit(state.saveData->items, LIST_POINTER);
}

inline GlobalState *GetState()
{
	return &state;
}

Item *GetItem()
{
	if (state.saveData->items.length != 0 && state.saveData->currentItem < state.saveData->items.length)
	{
		return ListGetPointer(state.saveData->items, state.saveData->currentItem);
	}
	return NULL;
}

void GiveItem(const ItemDefinition *definition, const bool switchToItem)
{
	if (switchToItem)
	{
		for (size_t i = 0; i < state.saveData->items.length; i++)
		{
			const Item *item = ListGetPointer(state.saveData->items, i);
			if (item->definition == definition)
			{
				SwitchToItem(definition);
				return;
			}
		}
	}
	Item *item = malloc(sizeof(Item));
	CheckAlloc(item);
	item->definition = definition;
	definition->Construct(item);
	ListAdd(state.saveData->items, item);
	if (switchToItem)
	{
		SwitchToItem(definition);
	}
}

void SwitchToItem(const ItemDefinition *definition)
{
	for (size_t i = 0; i < state.saveData->items.length; i++)
	{
		Item *item = ListGetPointer(state.saveData->items, i);
		if (item->definition == definition)
		{
			Item *previousItem = GetItem();
			state.saveData->currentItem = i;
			if (state.map)
			{
				if (previousItem)
				{
					previousItem->definition->SwitchFrom(previousItem, &state.map->viewmodel);
				}
				definition->SwitchTo(item, &state.map->viewmodel);
			}
			return;
		}
	}
	LogWarning("Was instructed to switch to an item that the player does not have!\n");
}

void NextItem()
{
	if (state.saveData->items.length == 0)
	{
		return;
	}
	if (state.saveData->currentItem < state.saveData->items.length - 1)
	{
		const Item *next = ListGetPointer(state.saveData->items, state.saveData->currentItem + 1);
		SwitchToItem(next->definition);
	}
}

void PreviousItem()
{
	if (state.saveData->items.length == 0)
	{
		return;
	}
	if (state.saveData->currentItem > 0)
	{
		const Item *next = ListGetPointer(state.saveData->items, state.saveData->currentItem - 1);
		SwitchToItem(next->definition);
	}
}

void SetGameState(const GameState *gameState)
{
	queuedStateChange = gameState;
	if (state.gameState == NULL)
	{
		ProcessStateChangeQueue();
	}
}

void ProcessStateChangeQueue()
{
	if (queuedStateChange)
	{
		if (state.gameState && state.gameState->Destroy)
		{
			state.gameState->Destroy();
		}
		if (queuedStateChange->Set)
		{
			queuedStateChange->Set();
		}
		state.gameState = queuedStateChange;
		PhysicsThreadSetFunction(queuedStateChange->FixedUpdateGame);
		DiscordUpdateRPC();
		if (!HasCliArg("--no-mouse-capture"))
		{
			SDL_SetWindowRelativeMouseMode(GetGameWindow(), queuedStateChange->enableRelativeMouseMode);
		}
		queuedStateChange = NULL;
	}
}

void ChangeMap(Map *map)
{
	state.camera = NULL;
	PhysicsThreadLockTickMutex();
	if (state.map)
	{
		DestroyMap(state.map);
	}
	state.map = map;
	state.camera = &state.map->player.playerCamera;
	PhysicsThreadUnlockTickMutex();
}

void DestroyGlobalState()
{
	LogDebug("Cleaning up GlobalState...\n");
	SaveOptions(&state.options);
	if (state.gameState->Destroy)
	{
		state.gameState->Destroy();
	}
	if (state.map)
	{
		DestroyMap(state.map);
	}
	for (size_t i = 0; i < state.saveData->items.length; i++)
	{
		Item *item = ListGetPointer(state.saveData->items, i);
		item->definition->Destruct(item);
		free(item);
	}
	ListFree(state.saveData->items);
	state.map = NULL;
	for (size_t i = 0; i < state.saveData->items.length; i++)
	{
		Item *item = ListGetPointer(state.saveData->items, i);
		item->definition->Destruct(item);
		free(item);
	}
	ListFree(state.saveData->items);
	free(state.saveData);

	LogDebug("Cleaning up physics...\n");
	PhysicsDestroyGlobal(&state);
}

bool ChangeMapByName(const char *name)
{
	LogInfo("Loading map \"%s\"\n", name);

	char mapPath[MAX_MAP_PATH_LENGTH];
	if (snprintf(mapPath, MAX_MAP_PATH_LENGTH, MAP("%s"), name) > MAX_MAP_PATH_LENGTH)
	{
		LogError("Failed to load map due to map name %s being too long\n", name);
		return false;
	}
	GetState()->saveData->blueCoins = 0;
	Map *map = CreateMap();
	ChangeMap(map);
	if (!LoadMap(map, DecompressAsset(mapPath, false, false)))
	{
		ChangeMap(NULL);
		return false;
	}
	map->mapName = strdup(name);
	DiscordUpdateRPC();
	return true;
}
