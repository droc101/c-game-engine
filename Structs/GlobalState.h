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
 * Update the volume from the options
 * @note This function should be called whenever the options are changed
 */
void UpdateVolume();

/**
 * Show a text box
 * @param tb Text box to show
 */
void ShowTextBox(TextBox tb);

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
 * Change the bgm
 * @param asset Asset to change to
 */
void ChangeMusic(const char *asset);

/**
 * Stop the bgm
 */
void StopMusic();

/**
 * Attempt to play a sound effect
 * @param asset Sound effect to play
 * @warning If there are no free channels, the sound effect will not play, and you will not be notified
 */
void PlaySoundEffect(const char *asset);

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

/**
 * Remove a trigger from a level
 * @param index The trigger to remove
 */
void RemoveTrigger(size_t index);

#endif //GAME_GLOBALSTATE_H
