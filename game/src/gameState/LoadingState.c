//
// Created by droc101 on 1/20/25.
//

#include "gameState/LoadingState.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/subsystem/Timing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include "gameState/MainState.h"

/// The minimum time the loading screen should be visible for, to prevent quick flashes
#define LEVEL_LOAD_MIN_TIME_MS 250

char loadStateLevelname[32];
uint64_t levelLoadStartTime;
bool loadStateLoadedLevel;

void GLoadingStateUpdate(GlobalState * /*state*/)
{
	if (!loadStateLoadedLevel)
	{
		loadStateLoadedLevel = true;
		const uint64_t realLoadStart = GetTimeNs();
		if (!ChangeLevelByName((char *)&loadStateLevelname))
		{
			LogError("Failed to load level: %s\n", loadStateLevelname);
		}
		const uint64_t realLoadEnd = GetTimeNs();
		const uint64_t realLoadTime = realLoadEnd - realLoadStart;
		LogInfo("Loaded level %s in %f ms\n", loadStateLevelname, (double)realLoadTime / 1000000.0);
	}
	const uint64_t currentTime = GetTimeMs();
	const uint64_t loadTime = currentTime - levelLoadStartTime;
	if (loadTime > LEVEL_LOAD_MIN_TIME_MS)
	{
		GMainStateSet();
	}
}

void GLoadingStateRender(GlobalState * /*state*/)
{
	DrawTextAligned("LOADING",
					16,
					COLOR_WHITE,
					v2s(0),
					v2(WindowWidthFloat(), WindowHeightFloat()),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					smallFont);
}

void GLoadingStateSet(const char *levelName)
{
	loadStateLoadedLevel = false;
	levelLoadStartTime = GetTimeMs();
	strncpy(loadStateLevelname, levelName, 31);
	StopMusic();
	SetStateCallbacks(GLoadingStateUpdate, NULL, GAME_STATE_LOADING, GLoadingStateRender, SDL_FALSE);
}
