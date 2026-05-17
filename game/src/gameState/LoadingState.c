//
// Created by droc101 on 1/20/25.
//

#include "gameState/LoadingState.h"
#include <assert.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/physics/MapPhysics.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/Timing.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "gameState/MainState.h"
#include "gameState/MenuState.h"

typedef enum LoadingStateStage
{
	/// Drawing the first frame ("LOADING" text)
	LSS_WAITING_FOR_FRAME,
	/// Loading the map from disk and performing the first frame update
	LSS_LOADING_LEVEL,
	/// Performing the first physics tick
	LSS_WAITING_FOR_TICK,
	/// Done, will chance to MainState once LEVEL_LOAD_MIN_TIME_MS check passes
	LSS_DONE
} LoadingStateStage;

/// The minimum time the loading screen should be visible for, to prevent quick flashes
#define LEVEL_LOAD_MIN_TIME_MS 250

char *loadStateLevelname = NULL;
uint64_t levelLoadStartTime;
LoadingStateStage stage;

void LoadingStateFixedUpdate(GlobalState *state, const double delta)
{
	if (stage == LSS_WAITING_FOR_TICK)
	{
		MapFixedUpdate(state, delta);
		stage = LSS_DONE;
	}
}

void LoadingStateUpdate(GlobalState *state, const double delta)
{
	if (stage == LSS_LOADING_LEVEL)
	{
		const uint64_t realLoadStart = GetTimeNs();
		if (!ChangeMapByName(loadStateLevelname))
		{
			LogError("Failed to load map: %s\n", loadStateLevelname);
			menuStateFadeIn = false;
			SetGameState(&MenuState); // get out before crash
		}
		const uint64_t realLoadEnd = GetTimeNs();
		const uint64_t realLoadTime = realLoadEnd - realLoadStart;
		LogInfo("Loaded map %s in %f ms\n", loadStateLevelname, (double)realLoadTime / 1000000.0);
		MapUpdate(state, delta);
		stage = LSS_WAITING_FOR_TICK;
	}
	const uint64_t currentTime = GetTimeMs();
	const uint64_t loadTime = currentTime - levelLoadStartTime;
	if (stage == LSS_DONE && loadTime > LEVEL_LOAD_MIN_TIME_MS)
	{
		SetGameState(&MainState);
	}
}

void LoadingStateRender(GlobalState * /*state*/, const double /*delta*/)
{
	DrawTextAligned("LOADING",
					16,
					COLOR_WHITE,
					v2s(0),
					v2(ScaledWindowWidthFloat(), ScaledWindowHeightFloat()),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					smallFont);
	if (stage == LSS_WAITING_FOR_FRAME)
	{
		stage = LSS_LOADING_LEVEL;
	}
}

void LoadingStateSet()
{
	levelLoadStartTime = GetTimeMs();
	assert(loadStateLevelname);
	stage = LSS_WAITING_FOR_FRAME;
}

void LoadingStateDestroy()
{
	free(loadStateLevelname);
}

const GameState LoadingState = {
	.UpdateGame = LoadingStateUpdate,
	.RenderGame = LoadingStateRender,
	.FixedUpdateGame = LoadingStateFixedUpdate,
	.Destroy = LoadingStateDestroy,
	.Set = LoadingStateSet,
	.enableRelativeMouseMode = false,
};
