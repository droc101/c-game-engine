//
// Created by droc101 on 4/22/2024.
//

#include "gameState/PauseState.h"
#include <SDL_gamecontroller.h>
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <engine/assets/AssetReader.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/subsystem/Discord.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/GlobalState.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/UiStack.h>
#include <engine/structs/Vector2.h>
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
		IsButtonJustPressed(SDL_CONTROLLER_BUTTON_START))
	{
		(void)PlaySoundEffect(SOUND("sfx/popdown"), 0, 1, NULL, NULL);
		MainStateSet();
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void PauseStateRender(GlobalState * /*state*/)
{
	RenderInGameMenuBackground();

	Vector2 logoPosition;
	Vector2 logoSize;
	logoPosition.x = ((float)WindowWidth() - 360) / 2;
	logoPosition.y = 32;
	logoSize.x = 360;
	logoSize.y = 240;
	DrawTexture(logoPosition, logoSize, TEXTURE("interface/pause_logo"));

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
					  SDL_FALSE); // Fixed update is not needed for this state
}

void PauseStateDestroy()
{
	if (pauseStack != NULL)
	{
		DestroyUiStack(pauseStack);
		pauseStack = NULL;
	}
}
