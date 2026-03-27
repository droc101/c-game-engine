//
// Created by droc101 on 1/20/25.
//

#include "gameState/LoadingState.h"
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/Timing.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "gameState/MainState.h"
#include "gameState/MenuState.h"

/// The minimum time the loading screen should be visible for, to prevent quick flashes
#define LEVEL_LOAD_MIN_TIME_MS 250

char *loadStateLevelname = NULL;
uint64_t levelLoadStartTime;
bool loadStateLoadedLevel;

void LoadingStateUpdate(GlobalState * /*state*/)
{
	if (!loadStateLoadedLevel)
	{
		loadStateLoadedLevel = true;
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
	}
	const uint64_t currentTime = GetTimeMs();
	const uint64_t loadTime = currentTime - levelLoadStartTime;
	if (loadTime > LEVEL_LOAD_MIN_TIME_MS)
	{
		SetGameState(&MainState);
	}
}

void LoadingStateRender(GlobalState * /*state*/)
{
	DrawTextAligned("LOADING",
					16,
					COLOR_WHITE,
					v2s(0),
					v2(ScaledWindowWidthFloat(), ScaledWindowHeightFloat()),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					smallFont);
}

void LoadingStateSet()
{
	loadStateLoadedLevel = false;
	levelLoadStartTime = GetTimeMs();
	assert(loadStateLevelname);
}

void LoadingStateDestroy() {
	free(loadStateLevelname);
}

const GameState LoadingState = {
	.UpdateGame = LoadingStateUpdate,
	.RenderGame = LoadingStateRender,
	.FixedUpdateGame = NULL,
	.Destroy = LoadingStateDestroy,
	.Set = LoadingStateSet,
	.enableRelativeMouseMode = false,
};
