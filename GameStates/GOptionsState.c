//
// Created by droc101 on 10/27/24.
//

#include "GOptionsState.h"
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/Color.h"
#include "../Structs/GlobalState.h"
#include "../Structs/UI/Controls/Button.h"
#include "../Structs/UI/UiStack.h"
#include "../Structs/Vector2.h"
#include "GMenuState.h"
#include "GPauseState.h"
#include "Options/GInputOptionsState.h"
#include "Options/GSoundOptionsState.h"
#include "Options/GVideoOptionsState.h"

UiStack *optionsStack = NULL;
bool optionsStateInGame = false;

void BtnOptionsBack()
{
	if (optionsStateInGame)
	{
		GPauseStateSet();
	} else
	{
		GMenuStateSet();
	}
}

void GOptionsStateUpdate(GlobalState * /*state*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(CONTROLLER_CANCEL))
	{
		BtnOptionsBack();
	}
}

void GOptionsStateRender(GlobalState * /*state*/)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground();
	}

	DrawTextAligned("Options",
					32,
					COLOR_WHITE,
					v2s(0),
					v2(WindowWidthFloat(), 100),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					largeFont);

	ProcessUiStack(optionsStack);
	DrawUiStack(optionsStack);
}

void GOptionsStateSet(const bool inGame)
{
	optionsStateInGame = inGame;
	if (optionsStack == NULL)
	{
		optionsStack = CreateUiStack();
		float opY = 80;
		const float opSpacing = 45;

		UiStackPush(optionsStack,
					CreateButtonControl(v2(0, opY), v2(480, 40), "Video Options", GVideoOptionsStateSet, TOP_CENTER));
		opY += opSpacing;
		UiStackPush(optionsStack,
					CreateButtonControl(v2(0, opY), v2(480, 40), "Sound Options", GSoundOptionsStateSet, TOP_CENTER));
		opY += opSpacing;
		UiStackPush(optionsStack,
					CreateButtonControl(v2(0, opY), v2(480, 40), "Input Options", GInputOptionsStateSet, TOP_CENTER));
		opY += opSpacing;

		UiStackPush(optionsStack, CreateButtonControl(v2(0, -40), v2(480, 40), "Done", BtnOptionsBack, BOTTOM_CENTER));
	}
	UiStackResetFocus(optionsStack);

	SetStateCallbacks(GOptionsStateUpdate,
					  NULL,
					  OPTIONS_STATE,
					  GOptionsStateRender); // Fixed update is not needed for this state
}

void GOptionsStateDestroy()
{
	if (optionsStack != NULL)
	{
		DestroyUiStack(optionsStack);
		optionsStack = NULL;
	}
}
