//
// Created by droc101 on 10/7/2025.
//

#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <engine/structs/ActorDefinition.h>
#include <stdbool.h>

#define STR(x) #x
#define TO_STR(x) STR(x)

#define ENGINE_VERSION_MAJOR 0
#define ENGINE_VERSION_MINOR 0
#define ENGINE_VERSION_PATCH 1
#define ENGINE_VERSION_SUFFIX "-levels-v2"
#define ENGINE_VERSION \
	TO_STR(ENGINE_VERSION_MAJOR) \
	"." TO_STR(ENGINE_VERSION_MINOR) "." TO_STR(ENGINE_VERSION_PATCH) ENGINE_VERSION_SUFFIX "-" CPU_TYPE

#ifdef WIN32
/// Make this symbol exported (in the symbol table)
#define EXPORT_SYM __declspec(dllexport)
#else
/// Make this symbol exported (in the symbol table)
#define EXPORT_SYM __attribute__((visibility("default")))
#endif

/**
 * Initialize the engine
 * @param argc Argument count
 * @param argv Argument values
 * @param RegisterGameActors Function to register all actors not defined in the engine
 */
void InitEngine(int argc, const char *argv[], RegisterGameActorsFunction RegisterGameActors);

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
