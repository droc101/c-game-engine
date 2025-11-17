//
// Created by droc101 on 4/22/2024.
//

#ifndef LOBALSTATE_H
#define LOBALSTATE_H

#include <engine/structs/Asset.h>
#include <engine/structs/Camera.h>
#include <engine/structs/Map.h>
#include <engine/structs/Options.h>
#include <engine/structs/Viewmodel.h>
#include <engine/subsystem/Discord.h>
#include <joltc/joltc.h>
#include <stdbool.h>
#include <stdint.h>

#define GAME_STATE_UNKNOWN 0

typedef uint64_t GameStateId;

typedef struct GlobalState GlobalState;
typedef struct SaveData SaveData;

typedef void (*FixedUpdateFunction)(GlobalState *state, double delta);

typedef void (*FrameUpdateFunction)(GlobalState *state);

typedef void (*FrameRenderFunction)(GlobalState *state);

struct SaveData
{
	/// Player health
	int hp;
	/// The number of coins the player has
	int coins;
	/// The number of blue coins the player has
	int blueCoins;
};

// Global state of the game
struct GlobalState
{
	/// Current map
	Map *map;

	JPH_JobSystem *jobSystem;

	/// State update function
	FrameUpdateFunction UpdateGame;
	/// State render function
	FrameRenderFunction RenderGame;
	/// The current state of the game
	GameStateId currentState;
	/// The number of physics frames that have passed since the last game state change
	uint64_t physicsFrame;

	/// The save data (persists between maps)
	SaveData *saveData;

	/// The camera
	Camera *camera;
	/// The scale of the UI.
	double uiScale;

	/// The view model
	Viewmodel viewmodel;

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

/**
 * Damage the player
 * @param damage Damage to deal
 */
void TakeDamage(int damage);

/**
 * Heal the player
 * @param amount Amount to heal
 */
void Heal(int amount);

/**
 * Set game state callbacks
 * @param UpdateGame update callback
 * @param FixedUpdateGame fixed-FPS update callback
 * @param currentState used for checking the state
 * @param RenderGame render callback
 * @param enableRelativeMouseMode
 */
void SetStateCallbacks(FrameUpdateFunction UpdateGame,
					   FixedUpdateFunction FixedUpdateGame,
					   GameStateId currentState,
					   FrameRenderFunction RenderGame,
					   SDL_bool enableRelativeMouseMode);

/**
 * Change the current map
 * @param map Map to change to
 * @param mapName
 */
void ChangeMap(Map *map, char *mapName);

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
