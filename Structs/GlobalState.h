//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_GLOBALSTATE_H
#define GAME_GLOBALSTATE_H

#include <joltc.h>
#include <stdbool.h>
#include <stdint.h>
#include "Camera.h"
#include "Level.h"
#include "Options.h"
#include "Viewmodel.h"

typedef enum CurrentState CurrentState;

typedef struct GlobalState GlobalState;
typedef struct SaveData SaveData;

typedef void (*FixedUpdateFunction)(GlobalState *state, double delta);

typedef void (*FrameUpdateFunction)(GlobalState *state);

typedef void (*FrameRenderFunction)(GlobalState *state);

/**
 * Used to check which game state the game is in
 * Now, you *could* just set a complete mess of state functions and disregard this, but if you do that, I will find you.
 */
enum CurrentState
{
	LEVEL_SELECT_STATE,
	LOGO_SPLASH_STATE,
	MAIN_STATE,
	MENU_STATE,
	PAUSE_STATE,
	OPTIONS_STATE,
	VIDEO_OPTIONS_STATE,
	SOUND_OPTIONS_STATE,
	INPUT_OPTIONS_STATE,
	LOADING_STATE,
};

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
	/// Current level
	Level *level;

	JPH_JobSystem *jobSystem;

	/// State update function
	FrameUpdateFunction UpdateGame;
	/// State render function
	FrameRenderFunction RenderGame;
	/// The current state of the game
	CurrentState currentState;
	/// The number of physics frames that have passed since the last game state change
	uint64_t physicsFrame;

	/// The save data (persists between levels)
	SaveData *saveData;

	/// The camera
	Camera *camera;
	/// The scale of the UI.
	double uiScale;

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
 */
void SetStateCallbacks(FrameUpdateFunction UpdateGame,
					   FixedUpdateFunction FixedUpdateGame,
					   CurrentState currentState,
					   FrameRenderFunction RenderGame);

/**
 * Change the current level
 * @param level Level to change to
 */
void ChangeLevel(Level *level);

/**
 * Destroy the global state
 */
void DestroyGlobalState();

/**
 * Change the level by name
 * @param name Level name to change to
 * @warning Don't use this from GMainState, use @c GLoadingSelectStateSet instead to avoid potential crashes
 */
bool ChangeLevelByName(const char *name);

#endif //GAME_GLOBALSTATE_H
