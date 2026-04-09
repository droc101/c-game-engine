//
// Created by droc101 on 4/22/2024.
//

#ifndef LOBALSTATE_H
#define LOBALSTATE_H

#include <engine/structs/Camera.h>
#include <engine/structs/Item.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Options.h>
#include <engine/subsystem/Discord.h>
#include <joltc/joltc.h>
#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <engine/structs/GameState.h>

typedef struct GlobalState GlobalState;
typedef struct SaveData SaveData;

struct SaveData
{
	/// Player health
	int hp;
	/// The number of coins the player has
	int coins;
	/// The number of blue coins the player has
	int blueCoins;
	/// The items the player has
	List items;
	size_t currentItem;
};

// Global state of the game
struct GlobalState
{
	/// Current map
	Map *map;

	JPH_JobSystem *jobSystem;

	const GameState *gameState;
	/// The number of physics frames that have passed since the last game state change
	uint64_t physicsFrame;

	/// The save data (persists between maps)
	SaveData *saveData;

	/// The camera
	Camera *camera;
	/// The scale of the UI.
	double uiScale;

	/// Game options
	Options options;

	/// The path to the executable
	char executablePath[261];
	/// The path to the executable folder
	char executableFolder[261];
	/// Whether to freeze the event loop. This should only be used for debugging.
	bool freezeEvents;
	/// Request to exit the game
	bool requestExit;

	/// The Discord RPC state
	RPCState rpcState;
};

/**
 * Load options
 * @warning This function should only be called once
 */
void InitOptions();

/**
 * Initialize the global state
 * @warning This function should only be called once
 */
void InitState();

/**
 * Get the global state
 * @return the global state
 */
GlobalState *GetState();

Item *GetItem();

bool HasItem(const ItemDefinition *definition);

void GiveItem(const ItemDefinition *definition, bool switchToItem);

void SwitchToItem(const ItemDefinition *definition);

void PreviousItem();

void NextItem();

/**
 * Set the game state
 */
void SetGameState(const GameState *gameState);

void ProcessStateChangeQueue();

/**
 * Change the current map
 * @param map Map to change to
 */
void ChangeMap(Map *map);

/**
 * Destroy the global state
 */
void DestroyGlobalState();

/**
 * Change the map by name
 * @param name Map name to change to
 * @warning Don't use this from MainState, use @c LoadingSelectStateSet instead to avoid potential crashes
 */
bool ChangeMapByName(const char *name);

#endif //LOBALSTATE_H
