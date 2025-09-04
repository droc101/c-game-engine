//
// Created by droc101 on 1/20/25.
//

#include "GLoadingState.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/SoundSystem.h"
#include "../Helpers/Core/Timing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/Color.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "GMainState.h"

/// The minimum time the loading screen should be visible for, to prevent quick flashes
#define LEVEL_LOAD_MIN_TIME_MS 250

char loadStateLevelname[32];
uint64_t levelLoadStartTime;

void GLoadingStateUpdate(GlobalState *state)
{
	if (state->physicsFrame == 1)
	{
		if (!ChangeLevelByName((char *)&loadStateLevelname))
		{
			LogError("Failed to load level: %s\n", loadStateLevelname);
		}
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
	levelLoadStartTime = GetTimeMs();
	strncpy(loadStateLevelname, levelName, 31);
	StopMusic();
	SetStateCallbacks(GLoadingStateUpdate, NULL, LOADING_STATE, GLoadingStateRender);
}
