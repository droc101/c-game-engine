//
// Created by droc101 on 3/27/26.
//

#ifndef GAME_GAMESTATE_H
#define GAME_GAMESTATE_H

#include <stdbool.h>

typedef struct GlobalState GlobalState; // forward decl.
typedef struct GameState GameState;

typedef void (*GameStateFixedUpdateFunction)(GlobalState *state, double delta);

typedef void (*GameStateFrameUpdateFunction)(GlobalState *state, double delta);

typedef void (*GameStateFrameRenderFunction)(GlobalState *state, double delta);

typedef void (*GameStateSetFunction)(void);

typedef void (*GameStateDestroyFunction)(void);

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
