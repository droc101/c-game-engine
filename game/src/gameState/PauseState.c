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
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/Button.h>
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

static void PauseStateRender(GlobalState *state, const double /*delta*/)
{
	RenderInGameMenuBackground();

	Vector2 logoPosition;
	Vector2 logoSize;
	logoPosition.x = ((float)ScaledWindowWidth() - 360) / 2;
	logoPosition.y = 32;
	logoSize.x = 360;
	logoSize.y = 240;
	DrawTexture(logoPosition, logoSize, TEXTURE("interface/pause_logo"));

	ProcessUiStack(pauseStack);
	DrawUiStack(pauseStack);
}

static void BtnPauseResume()
{
	SetGameState(&MainState);
}

static void BtnOptions()
{
	optionsStateInGame = true;
	SetGameState(&OptionsState);
}

static void BtnPauseExit()
{
	ChangeMap(NULL);
	SetGameState(&LevelSelectState);
}

static void PauseStateSet()
{
	GetState()->rpcState = PAUSED;
	if (pauseStack == NULL)
	{
		pauseStack = CreateUiStack();
		UiStackPush(pauseStack,
					CreateButtonControl(v2(0, 20), v2(300, 40), "Resume", BtnPauseResume, MIDDLE_CENTER, NULL));
		UiStackPush(pauseStack,
					CreateButtonControl(v2(0, 70), v2(300, 40), "Options", BtnOptions, MIDDLE_CENTER, NULL));
		UiStackPush(pauseStack,
					CreateButtonControl(v2(0, 120), v2(300, 40), "Exit Level", BtnPauseExit, MIDDLE_CENTER, NULL));
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
