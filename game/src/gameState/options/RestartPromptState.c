//
// Created by droc101 on 8/2/26.
//

#include "gameState/options/RestartPromptState.h"
#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include "gameState/OptionsState.h"

static UiStack *menuStack = NULL;

static void BtnDontRelaunch(Control *, void *)
{
	SetGameState(&OptionsState);
}

static void BtnRelaunch(Control *, void *)
{
	RestartProgram();
}

static void RestartPromptStateUpdate(GlobalState *state, const double delta)
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL))
	{
		BtnDontRelaunch(NULL, NULL);
	}
	if (!optionsStateInGame)
	{
		UpdateMenuBackground(state, delta);
	}
}

static void RestartPromptStateRender(GlobalState *state, const double /*delta*/)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground(state, true);
	}

	ProcessUiStack(menuStack);
	DrawUiStack(menuStack);
}

static void RestartPromptStateSet()
{
	GetState()->rpcState = IN_MENUS;
	if (menuStack == NULL)
	{
		menuStack = CreateUiStack();

		UiStackPush(menuStack,
					CreateLabelControl("Restart Required",
									   32,
									   COLOR_WHITE,
									   v2(0, -70),
									   v2(550, 40),
									   MIDDLE_CENTER,
									   FONT_HALIGN_CENTER,
									   FONT_VALIGN_MIDDLE,
									   FONT("large_font"),
									   false));
		UiStackPush(menuStack,
					CreateLabelControl("You have changed options that require a relaunch. Would you like to relaunch "
									   "now?",
									   16,
									   COLOR_WHITE,
									   v2(0, -30),
									   v2(DEF_WIDTH, 40),
									   MIDDLE_CENTER,
									   FONT_HALIGN_CENTER,
									   FONT_VALIGN_MIDDLE,
									   FONT("small_font"),
									   true));

		UiStackPush(menuStack, CreateButtonControl(v2(-190, 50), v2(370, 40), "Yes", BtnRelaunch, MIDDLE_CENTER, NULL));
		UiStackPush(menuStack,
					CreateButtonControl(v2(190, 50), v2(370, 40), "No", BtnDontRelaunch, MIDDLE_CENTER, NULL));
	}
	UiStackResetFocus(menuStack);

	EnterMenuBackgroundState();
}

static void RestartPromptStateDestroy()
{
	if (menuStack != NULL)
	{
		DestroyUiStack(menuStack);
		menuStack = NULL;
	}
}

const GameState RestartPromptState = {
	.UpdateGame = RestartPromptStateUpdate,
	.RenderGame = RestartPromptStateRender,
	.FixedUpdateGame = FixedUpdateMenuBackground,
	.Destroy = RestartPromptStateDestroy,
	.Set = RestartPromptStateSet,
	.enableRelativeMouseMode = false,
};
