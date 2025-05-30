//
// Created by droc101 on 1/20/25.
//

#include "GLoadingState.h"
#include <string.h>
#include "../../../Helpers/CommonAssets.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "GMainState.h"

char loadStateLevelname[32];

void GLoadingStateUpdate(GlobalState *state)
{
	if (state->physicsFrame >= 15)
	{
		if (ChangeLevelByName((char *)&loadStateLevelname))
		{
			GMainStateSet();
		} else
		{
			LogError("Failed to load level: %s\n", loadStateLevelname);
		}
	}
}

void GLoadingStateRender(GlobalState *)
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
	strncpy(loadStateLevelname, levelName, 31);
	StopMusic();
	SetStateCallbacks(GLoadingStateUpdate, NULL, LOADING_STATE, GLoadingStateRender);
}
