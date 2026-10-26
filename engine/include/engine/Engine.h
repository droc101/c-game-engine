//
// Created by droc101 on 10/7/2025.
//

/**
 * @file Engine.h
 * Core engine mainloop
 */

#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <engine/Commit.h>
#include <engine/gameState/LoadingState.h>
#include <engine/structs/Achievements.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/ControlOptions.h>
#include <stdbool.h>

/// FPS target, used for delta calculations
#define TARGET_FPS 60
/// Target nanoseconds per frame, used for delta calculations
#define TARGET_FPS_NS_D (1000000000.0 / TARGET_FPS)

#define STR(x) #x
#define TO_STR(x) STR(x)

#define ENGINE_VERSION_MAJOR 0
#define ENGINE_VERSION_MINOR 0
#define ENGINE_VERSION_PATCH 1
#define ENGINE_VERSION_SUFFIX "-brushes"
/// Engine version string
#define ENGINE_VERSION \
	TO_STR(ENGINE_VERSION_MAJOR) \
	"." TO_STR(ENGINE_VERSION_MINOR) "." TO_STR(ENGINE_VERSION_PATCH) ENGINE_VERSION_SUFFIX \
			"-" CPU_TYPE " (" ENGINE_GIT_SHORT_HASH ")"

#ifdef WIN32
/// Make this symbol exported (in the symbol table)
#define EXPORT_SYM __declspec(dllexport)
#else
/// Make this symbol exported (in the symbol table)
#define EXPORT_SYM __attribute__((visibility("default")))
#endif

/// Engine startup configuration
typedef struct EngineInitializationInfo
{
	/// Number of process arguments
	const int argc;
	/// Process arguments
	char **argv;
	/// Game-side actor registration function
	const RegisterGameActorsFunction RegisterGameActors;
	/// Game-side control registration function
	const ControlRegisterFunction RegisterGameControls;
	/// Game-side achievement and statistic registration function
	const RegisterGameAchievementsFunction RegisterGameAchievements;
	/// Default game assets path
	const char *defaultGamePath;
	/// Loading state map loaded callback
	const LoadingStateDoneFunction LoadingStateDoneCallback;
	/// Loading state load error callback
	const LoadingStateErrorFunction LoadingStateErrorCallback;
} EngineInitializationInfo;

/**
 * Initialize the engine
 * @param initInfo Engine configuration and initialization parameters
 */
void InitEngine(EngineInitializationInfo initInfo);

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

#endif //GAME_ENGINE_H
