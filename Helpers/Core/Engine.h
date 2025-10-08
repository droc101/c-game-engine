//
// Created by droc101 on 10/7/2025.
//

#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <SDL_events.h>
#include <stdbool.h>

/**
 * Initialize the engine
 * @param argc Argument count
 * @param argv Argument values
 */
void InitEngine(int argc, const char *argv[]);

/**
 * Perform an iteration of the engine's main loop
 */
void EngineIteration();

/**
 * Destroy the engine
 */
void DestroyEngine();

/**
 * Check if the engine's main loop should continue
 */
bool EngineShouldQuit();

//
// PRIVATE FUNCTIONS
//

/**
 * Check if the system's CPU supports the required features.
 */
bool CheckCPUSupport();

/**
 * Attempt to initialize the executable path
 * @param argc Program argument count
 * @param argv Program arguments
 */
void ExecPathInit(int argc, const char *argv[]);

/**
 * Initialize the SDL library
 */
void InitSDL();

/**
 * Initialize the game window and renderer
 */
void WindowAndRenderInit();

/**
 * Handle an SDL event
 */
void HandleEvent(void);

#endif //GAME_ENGINE_H
