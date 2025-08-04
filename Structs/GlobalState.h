//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_GLOBALSTATE_H
#define GAME_GLOBALSTATE_H

#include "../defines.h"

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
