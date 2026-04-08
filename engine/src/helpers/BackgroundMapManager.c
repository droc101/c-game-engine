//
// Created by droc101 on 4/7/26.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/Arguments.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/physics/MapPhysics.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Map.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/Timing.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

size_t bgMapLoadFrameCounter = 0;
float placeholderOpacity = 1.0f;

static bool IsBackgroundMapLoadedIgnoreTicks()
{
	return GetState()->map &&
		   GetState()->map->mapName &&
		   strcmp(GetState()->map->mapName, gameConfig.backgroundMap) == 0;
}

bool IsBackgroundMapLoaded()
{
	return IsBackgroundMapLoadedIgnoreTicks() && GetState()->map->physicsTick > 0;
}

void EnterMenuBackgroundState()
{
	if (!IsBackgroundMapLoaded())
	{
		bgMapLoadFrameCounter = 0;
		placeholderOpacity = 1.0f;
	}
}

void UpdateMenuBackground(GlobalState *state)
{
	if (HasCliArg("--no-background-map"))
	{
		ChangeMap(NULL);
	} else
	{
		if (!IsBackgroundMapLoadedIgnoreTicks())
		{
			if (bgMapLoadFrameCounter > 10)
			{
				const uint64_t realLoadStart = GetTimeNs();
				if (!ChangeMapByName(gameConfig.backgroundMap))
				{
					LogError("Failed to load background map: %s\n", gameConfig.backgroundMap);
				}
				const uint64_t realLoadEnd = GetTimeNs();
				const uint64_t realLoadTime = realLoadEnd - realLoadStart;
				LogInfo("Loaded background map %s in %f ms\n",
						gameConfig.backgroundMap,
						(double)realLoadTime / 1000000.0);
				MapUpdate(state);
			}
		}
	}
}

void RenderMenuBackground(GlobalState *state)
{
	if (IsBackgroundMapLoaded())
	{
		RenderMap(state->map, state->camera);
		const uint32_t alpha = (uint32_t)(0xFF * placeholderOpacity) << 24;
		DrawTextureMod(v2s(0),
					   v2(ScaledWindowWidth(), ScaledWindowHeight()),
					   TEXTURE("interface/background_placeholder"),
					   &COLOR(0x00FFFFFF | alpha));
	} else
	{
		DrawTexture(v2s(0), v2(ScaledWindowWidth(), ScaledWindowHeight()), TEXTURE("interface/background_placeholder"));
		DrawTextAligned("LOADING",
						16,
						COLOR_BLACK,
						v2s(2),
						v2(ScaledWindowWidthFloat(), ScaledWindowHeightFloat()),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						smallFont);
		DrawTextAligned("LOADING",
						16,
						COLOR_WHITE,
						v2s(0),
						v2(ScaledWindowWidthFloat(), ScaledWindowHeightFloat()),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						smallFont);
	}
	bgMapLoadFrameCounter++;
}

void FixedUpdateMenuBackground(GlobalState *state, const double delta)
{
	if (IsBackgroundMapLoadedIgnoreTicks())
	{
		MapFixedUpdate(state, delta);
		placeholderOpacity -= (float)(delta * 0.01);
		placeholderOpacity = fmaxf(placeholderOpacity, 0);
	}
}
