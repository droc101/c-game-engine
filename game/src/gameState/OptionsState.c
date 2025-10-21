//
// Created by droc101 on 10/27/24.
//

#include "gameState/OptionsState.h"
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <engine/subsystem/Input.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/UiStack.h>
#include <engine/structs/Vector2.h>
#include "gameState/MenuState.h"
#include "gameState/PauseState.h"
#include "gameState/options/InputOptionsState.h"
#include "gameState/options/SoundOptionsState.h"
#include "gameState/options/VideoOptionsState.h"

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
					  GAME_STATE_OPTIONS,
					  GOptionsStateRender,
					  SDL_FALSE); // Fixed update is not needed for this state
}

void GOptionsStateDestroy()
{
	if (optionsStack != NULL)
	{
		DestroyUiStack(optionsStack);
		optionsStack = NULL;
	}
}
