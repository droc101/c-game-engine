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
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/Image.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/UiStack.h>
#include <engine/uiStack/UiTheme.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include "gameState/AchievementsState.h"
#include "gameState/AddonsState.h"
#include "gameState/LevelSelectState.h"
#include "gameState/OptionsState.h"

static UiStack *menuStack = NULL;
bool menuStateFadeIn = false;
static bool easterEgg = false;
static char versionStringBuffer[256];

static void StartGame(Control * /*button*/, void * /*extraData*/)
{
	SetGameState(&LevelSelectState);
}

static void QuitGame(Control * /*button*/, void * /*extraData*/)
{
	GetState()->requestExit = true;
}

static void OpenOptions(Control * /*button*/, void * /*extraData*/)
{
	optionsStateInGame = false;
	SetGameState(&OptionsState);
}

static void OpenAddons(Control * /*button*/, void * /*extraData*/)
{
	SetGameState(&AddonsState);
}

static void OpenAchievements(Control * /*button*/, void * /*extraData*/)
{
	SetGameState(&AchievementsState);
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
									   &uiTheme.secondaryText,
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
										   &uiTheme.primaryText,
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
		UiStackPush(menuStack,
					CreateButtonControl(v2(-122.5, opY), v2(235, 40), "Addons", OpenAddons, MIDDLE_CENTER, NULL));
		UiStackPush(menuStack,
					CreateButtonControl(v2(122.5, opY),
										v2(235, 40),
										"Achievements",
										OpenAchievements,
										MIDDLE_CENTER,
										NULL));
		opY += opSpacing * 1.5f;
		UiStackPush(menuStack, CreateButtonControl(v2(0, opY), v2(480, 40), "Quit", QuitGame, MIDDLE_CENTER, NULL));
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
