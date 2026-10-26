//
// Created by droc101 on 3/27/26.
//

/**
 * @file GameState.h
 * GameState definitions
 */

#ifndef GAME_GAMESTATE_H
#define GAME_GAMESTATE_H

#include <stdbool.h>

typedef struct GlobalState GlobalState; // forward decl.
typedef struct GameState GameState;

/**
 * Function signature for GameState physics update
 * @param state GlobalState
 * @param delta Physics delta
 */
typedef void (*GameStateFixedUpdateFunction)(GlobalState *state, double delta);

/**
 * Function signature for GameState frame update
 * @param state GlobalState
 * @param delta Frame delta
 */
typedef void (*GameStateFrameUpdateFunction)(GlobalState *state, double delta);

/**
 * Function signature for GameState rendering
 * @param state GlobalState
 * @param delta Frame delta
 */
typedef void (*GameStateFrameRenderFunction)(GlobalState *state, double delta);

/// Function signature for GameState set/initialize
typedef void (*GameStateSetFunction)(void);

/// Function signature for GameState leave/destroy
typedef void (*GameStateDestroyFunction)(void);

/// GameState definition
struct GameState
{
	/// State update function
	GameStateFrameUpdateFunction UpdateGame;
	/// State render function
	GameStateFrameRenderFunction RenderGame;
	/// Physics state update function
	GameStateFixedUpdateFunction FixedUpdateGame;
	/// Constructor/Initialization function
	GameStateSetFunction Set;
	/// Destructor/Cleanup function
	GameStateDestroyFunction Destroy;
	/// Whether to enable relative mouse mode
	bool enableRelativeMouseMode;
};

#endif //GAME_GAMESTATE_H
