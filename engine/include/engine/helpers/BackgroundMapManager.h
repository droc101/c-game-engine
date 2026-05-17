//
// Created by droc101 on 4/7/26.
//

#ifndef GAME_BACKGROUNDMAPMANAGER_H
#define GAME_BACKGROUNDMAPMANAGER_H

#include <engine/structs/GlobalState.h>
#include <stdbool.h>

/**
 * Check if the background map has been fully loaded
 */
bool IsBackgroundMapLoaded();

/**
 * Reset the background map loading system
 */
void EnterMenuBackgroundState();

/**
 * Perform a frame update of the background map
 * @param state GlobalState pointer
 * @param delta Delta time
 */
void UpdateMenuBackground(GlobalState *state, double delta);

/**
 * Render the menu background map
 * @param state GlobalState pointer
 */
void RenderMenuBackground(GlobalState *state);

/**
 * Perform a physics tick update of the background map
 * @param state GlobalState pointer
 * @param delta Delta time
 */
void FixedUpdateMenuBackground(GlobalState *state, double delta);

#endif //GAME_BACKGROUNDMAPMANAGER_H
