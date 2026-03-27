//
// Created by droc101 on 3/27/26.
//

#ifndef GAME_GAMESTATE_H
#define GAME_GAMESTATE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct GlobalState GlobalState; // forward decl.
typedef struct GameState GameState;

typedef void (*GameStateFixedUpdateFunction)(GlobalState *state, double delta);

typedef void (*GameStateFrameUpdateFunction)(GlobalState *state);

typedef void (*GameStateFrameRenderFunction)(GlobalState *state);

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
	GameStateSetFunction Set;
	GameStateDestroyFunction Destroy;
	/// Whether to enable relative mouse mode
	bool enableRelativeMouseMode;
};

#endif //GAME_GAMESTATE_H
