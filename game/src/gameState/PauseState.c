//
// Created by droc101 on 4/22/2024.
//

#include "gameState/PauseState.h"
#include <engine/assets/AssetReader.h>
#include <engine/debug/DPrint.h>
#include <engine/Engine.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>
#include <stddef.h>
#include "gameState/MainState.h"
#include "gameState/MenuState.h"
#include "gameState/OptionsState.h"

#ifdef USE_LEVEL_SELECT
#include "gameState/LevelSelectState.h"
#endif

UiStack *pauseStack = NULL;

void PauseStateUpdate(GlobalState * /*state*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(CONTROLLER_CANCEL) ||
		IsButtonJustPressed(SDL_GAMEPAD_BUTTON_START))
	{
		(void)PlaySound(SOUND("sfx/popdown"), 0, 1, NULL, NULL);
		MainStateSet();
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void PauseStateRender(GlobalState * /*state*/)
{
	RenderInGameMenuBackground();

	Vector2 logoPosition;
	Vector2 logoSize;
	logoPosition.x = ((float)ScaledWindowWidth() - 360) / 2;
	logoPosition.y = 32;
	logoSize.x = 360;
	logoSize.y = 240;
	DrawTexture(logoPosition, logoSize, TEXTURE("interface/pause_logo"));

#ifdef BUILDSTYLE_DEBUG
	DPrintF("Engine " ENGINE_VERSION "\nDEBUG BUILD", COLOR_WHITE, false);
#endif

	ProcessUiStack(pauseStack);
	DrawUiStack(pauseStack);
}

void BtnPauseResume()
{
	MainStateSet();
}

void BtnOptions()
{
	OptionsStateSet(true);
}

void BtnPauseExit()
{
#ifdef USE_LEVEL_SELECT
	LevelSelectStateSet();
#else
	MenuStateSet();
#endif
}

void PauseStateSet()
{
	GetState()->rpcState = PAUSED;
	if (pauseStack == NULL)
	{
		pauseStack = CreateUiStack();
		UiStackPush(pauseStack, CreateButtonControl(v2(0, 20), v2(300, 40), "Resume", BtnPauseResume, MIDDLE_CENTER));
		UiStackPush(pauseStack, CreateButtonControl(v2(0, 70), v2(300, 40), "Options", BtnOptions, MIDDLE_CENTER));
		UiStackPush(pauseStack,
					CreateButtonControl(v2(0, 120), v2(300, 40), "Exit Level", BtnPauseExit, MIDDLE_CENTER));
	}
	UiStackResetFocus(pauseStack);

	SetStateCallbacks(PauseStateUpdate,
					  NULL,
					  GAME_STATE_PAUSE,
					  PauseStateRender,
					  false); // Fixed update is not needed for this state
}

void PauseStateDestroy()
{
	if (pauseStack != NULL)
	{
		DestroyUiStack(pauseStack);
		pauseStack = NULL;
	}
}
