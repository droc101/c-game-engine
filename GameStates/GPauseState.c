//
// Created by droc101 on 4/22/2024.
//

#include "GPauseState.h"
#include <stdio.h>
#include "../../../Structs/Vector2.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/GlobalState.h"
#include "../Structs/UI/Controls/Button.h"
#include "../Structs/UI/UiStack.h"
#include "GLevelSelectState.h"
#include "GMainState.h"
#include "GOptionsState.h"

#ifndef USE_LEVEL_SELECT
#include "GMenuState.h"
#endif

UiStack *pauseStack = NULL;

void GPauseStateUpdate(GlobalState * /*State*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(CONTROLLER_CANCEL) ||
		IsButtonJustPressed(SDL_CONTROLLER_BUTTON_START))
	{
		PlaySoundEffect(SOUND("sfx_popdown"));
		GMainStateSet();
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GPauseStateRender(GlobalState *State)
{
	RenderInGameMenuBackground();

	DrawTextAligned("Game Paused",
					32,
					COLOR_WHITE,
					v2s(0),
					v2(WindowWidthFloat(), 250),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					largeFont);

	const char *levelID = State->level->name;
	const int cNum = State->level->courseNum;

	if (cNum != -1)
	{
		char buf[64];
		sprintf(buf, "Level %d", cNum);
		DrawTextAligned(buf,
						16,
						COLOR_BLACK,
						v2(4, 164),
						v2(WindowWidthFloat(), 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						smallFont);
		DrawTextAligned(buf,
						16,
						COLOR_WHITE,
						v2(0, 160),
						v2(WindowWidthFloat(), 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						smallFont);
	}

	DrawTextAligned(levelID,
					32,
					COLOR_BLACK,
					v2(4, 204),
					v2(WindowWidthFloat(), 40),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					smallFont);
	DrawTextAligned(levelID,
					32,
					COLOR_WHITE,
					v2(0, 200),
					v2(WindowWidthFloat(), 40),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					smallFont);

	ProcessUiStack(pauseStack);
	DrawUiStack(pauseStack);
}

void BtnPauseResume()
{
	GMainStateSet();
}

void BtnOptions()
{
	GOptionsStateSet(true);
}

void BtnPauseExit()
{
#ifdef USE_LEVEL_SELECT
	GLevelSelectStateSet();
#else
	GMenuStateSet();
#endif
}

void GPauseStateSet()
{
	if (pauseStack == NULL)
	{
		pauseStack = CreateUiStack();
		UiStackPush(pauseStack, CreateButtonControl(v2(0, 20), v2(300, 40), "Resume", BtnPauseResume, MIDDLE_CENTER));
		UiStackPush(pauseStack, CreateButtonControl(v2(0, 70), v2(300, 40), "Options", BtnOptions, MIDDLE_CENTER));
		UiStackPush(pauseStack,
					CreateButtonControl(v2(0, 120), v2(300, 40), "Exit Level", BtnPauseExit, MIDDLE_CENTER));
	}
	UiStackResetFocus(pauseStack);

	SetStateCallbacks(GPauseStateUpdate,
					  NULL,
					  PAUSE_STATE,
					  GPauseStateRender); // Fixed update is not needed for this state
}

void GPauseStateDestroy()
{
	if (pauseStack != NULL)
	{
		DestroyUiStack(pauseStack);
		pauseStack = NULL;
	}
}
