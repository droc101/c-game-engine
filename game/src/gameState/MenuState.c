//
// Created by droc101 on 4/22/2024.
//

#include "gameState/MenuState.h"
#include <engine/assets/AssetReader.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/Engine.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/Arguments.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Logging.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/Image.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/UiStack.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "gameState/LevelSelectState.h"
#include "gameState/OptionsState.h"

static UiStack *menuStack = NULL;
bool menuStateFadeIn = false;
static bool easterEgg = false;
static char versionStringBuffer[256];

static void StartGame(Control *, void *)
{
	SetGameState(&LevelSelectState);
}

static void QuitGame(Control *, void *)
{
	GetState()->requestExit = true;
}

static void OpenOptions(Control *, void *)
{
	optionsStateInGame = false;
	SetGameState(&OptionsState);
}

static void ReloadAssets(Control *, void *)
{
	LogInfo("Reloading all assets\n");
	ChangeMap(NULL);
	EnterMenuBackgroundState();
	rendererQueuedActions |= QUEUED_ACTION_RELOAD_ALL_ASSETS;
}

static void MenuStateRender(GlobalState *state, const double /*delta*/)
{
	RenderMenuBackground(state, false);
	if (!IsBackgroundMapLoaded())
	{
		return;
	}

	ProcessUiStack(menuStack);
	DrawUiStack(menuStack);
}

static void MenuStateSet()
{
	GetState()->rpcState = IN_MENUS;
	if (menuStack == NULL)
	{
		const time_t current = time(NULL);
		const struct tm *t = localtime(&current);
		easterEgg = (t->tm_mon == 3 && t->tm_mday == 1) || HasCliArg("--force-menu-easter-egg");

		menuStack = CreateUiStack();

		sprintf(versionStringBuffer, "Engine %s\n%s", ENGINE_VERSION, gameConfig.gameCopyright);
		UiStackPush(menuStack,
					CreateLabelControl(versionStringBuffer,
									   16,
									   COLOR(0xFFa0a0a0),
									   v2s(0),
									   v2(DEF_WIDTH, 60),
									   BOTTOM_CENTER,
									   FONT_HALIGN_CENTER,
									   FONT_VALIGN_MIDDLE,
									   FONT("small_font"),
									   true));
		UiStackPush(menuStack,
					CreateImageControl(v2(0, 32), v2(480, 320), TEXTURE("interface/menu_logo"), TOP_CENTER, NULL));

		if (easterEgg)
		{
			UiStackPush(menuStack,
						CreateLabelControl("the",
										   64,
										   COLOR_WHITE,
										   v2(0, 16),
										   v2(480, 64),
										   TOP_CENTER,
										   FONT_HALIGN_CENTER,
										   FONT_VALIGN_MIDDLE,
										   FONT("large_font"),
										   false));
		}

		float opY = 80;
		const float opSpacing = 50;

		UiStackPush(menuStack, CreateButtonControl(v2(0, opY), v2(480, 40), "Start", StartGame, MIDDLE_CENTER, NULL));
		opY += opSpacing;
		UiStackPush(menuStack,
					CreateButtonControl(v2(0, opY), v2(480, 40), "Options", OpenOptions, MIDDLE_CENTER, NULL));
		opY += opSpacing;
		UiStackPush(menuStack, CreateButtonControl(v2(0, opY), v2(480, 40), "Quit", QuitGame, MIDDLE_CENTER, NULL));
		opY += opSpacing * 1.5;
		UiStackPush(menuStack,
					CreateButtonControl(v2(0, opY),
										v2(480, 40),
										"Hot Reload Assets",
										ReloadAssets,
										MIDDLE_CENTER,
										"top 10 ways to kill the engine"));
		opY += opSpacing;
	}
	UiStackResetFocus(menuStack);
	EnterMenuBackgroundState();
}

static void MenuStateDestroy()
{
	if (menuStack != NULL)
	{
		DestroyUiStack(menuStack);
		menuStack = NULL;
	}
}

const GameState MenuState = {
	.UpdateGame = UpdateMenuBackground,
	.RenderGame = MenuStateRender,
	.FixedUpdateGame = FixedUpdateMenuBackground,
	.Destroy = MenuStateDestroy,
	.Set = MenuStateSet,
	.enableRelativeMouseMode = false,
};
