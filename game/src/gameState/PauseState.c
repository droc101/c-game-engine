//
// Created by droc101 on 4/22/2024.
//

#include "gameState/PauseState.h"
#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/Image.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include "gameState/LevelSelectState.h"
#include "gameState/MainState.h"
#include "gameState/OptionsState.h"

static UiStack *pauseStack = NULL;

static void PauseStateUpdate(GlobalState * /*state*/, const double /*delta*/)
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL) ||
		IsButtonJustPressed(mainThreadInput, SDL_GAMEPAD_BUTTON_START))
	{
		(void)PlaySound(SOUND("sfx/popdown"), SOUND_CATEGORY_UI);
		SetGameState(&MainState);
	}
}

static void PauseStateRender(GlobalState * /*state*/, const double /*delta*/)
{
	RenderInGameMenuBackground();
	ProcessUiStack(pauseStack);
	DrawUiStack(pauseStack);
}

static void BtnPauseResume(Control *, void *)
{
	SetGameState(&MainState);
}

static void BtnOptions(Control *, void *)
{
	optionsStateInGame = true;
	SetGameState(&OptionsState);
}

static void BtnPauseExit(Control *, void *)
{
	ChangeMap(NULL);
	SetGameState(&LevelSelectState);
}

static void BtnHotReloadAssets(Control *, void *)
{
	LogInfo("Reloading all assets\n");
	ReloadMap();
	rendererQueuedActions |= QUEUED_ACTION_RELOAD_ALL_ASSETS;
}

static void PauseStateSet()
{
	GetState()->rpcState = PAUSED;
	if (pauseStack == NULL)
	{
		pauseStack = CreateUiStack();

		UiStackPush(pauseStack,
					CreateImageControl(v2(0, 32), v2(360, 240), TEXTURE("interface/pause_logo"), TOP_CENTER, NULL));

		UiStackPush(pauseStack,
					CreateButtonControl(v2(0, 20), v2(300, 40), "Resume", BtnPauseResume, MIDDLE_CENTER, NULL));
		UiStackPush(pauseStack,
					CreateButtonControl(v2(0, 70), v2(300, 40), "Options", BtnOptions, MIDDLE_CENTER, NULL));
		UiStackPush(pauseStack,
					CreateButtonControl(v2(0, 120), v2(300, 40), "Exit Level", BtnPauseExit, MIDDLE_CENTER, NULL));
		UiStackPush(pauseStack,
					CreateButtonControl(v2(0, 195),
										v2(300, 40),
										"Hot Reload Assets",
										BtnHotReloadAssets,
										MIDDLE_CENTER,
										"top 9 ways to kill the engine"));
	}
	UiStackResetFocus(pauseStack);
}

static void PauseStateDestroy()
{
	if (pauseStack != NULL)
	{
		DestroyUiStack(pauseStack);
		pauseStack = NULL;
	}
}

const GameState PauseState = {
	.UpdateGame = PauseStateUpdate,
	.RenderGame = PauseStateRender,
	.FixedUpdateGame = NULL,
	.Set = PauseStateSet,
	.Destroy = PauseStateDestroy,
	.enableRelativeMouseMode = false,
};
