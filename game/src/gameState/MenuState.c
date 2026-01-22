//
// Created by droc101 on 4/22/2024.
//

#include "gameState/MenuState.h"
#include <engine/assets/AssetReader.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/Engine.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/Arguments.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "gameState/LevelSelectState.h"
#include "gameState/OptionsState.h"

#ifndef USE_LEVEL_SELECT
#include "gameState/MainState.h"
#endif

UiStack *menuStack = NULL;
bool fadeIn = false;
bool easterEgg = false;

void StartGame()
{
#ifdef USE_LEVEL_SELECT
	LevelSelectStateSet();
#else
	MainStateSet();
#endif
}

void QuitGame()
{
	GetState()->requestExit = true;
}

void OpenOptions()
{
	OptionsStateSet(false);
}

void MenuStateUpdate(GlobalState * /*state*/) {}

void MenuStateRender(GlobalState *state)
{
	RenderMenuBackground();

	// draw the logo
	Vector2 logoPosition;
	Vector2 logoSize;
	logoPosition.x = ((float)ScaledWindowWidth() - 480) / 2;
	logoPosition.y = 32;
	logoSize.x = 480;
	logoSize.y = 320;
	DrawTexture(logoPosition, logoSize, TEXTURE("interface/menu_logo"));

	if (easterEgg)
	{
		logoPosition.y -= 16;
		DrawTextAligned("the",
						64,
						COLOR_WHITE,
						logoPosition,
						v2(480, 64),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						largeFont);
	}

#ifdef BUILDSTYLE_DEBUG
	FontDrawString(v2(20, 20), "DEBUG BUILD", 16, COLOR(0xFF00FF00), smallFont);
#endif

	// draw version and copyright info
	char buffer[256];
	sprintf(buffer, "Engine %s\n%s", ENGINE_VERSION, gameConfig.gameCopyright);
	DrawTextAligned(buffer,
					16,
					COLOR_BLACK,
					v2(ScaledWindowWidthFloat() - 208, ScaledWindowHeightFloat() - 208),
					v2(200, 200),
					FONT_HALIGN_RIGHT,
					FONT_VALIGN_BOTTOM,
					smallFont);
	DrawTextAligned(buffer,
					16,
					COLOR(0xFFa0a0a0),
					v2(ScaledWindowWidthFloat() - 210, ScaledWindowHeightFloat() - 210),
					v2(200, 200),
					FONT_HALIGN_RIGHT,
					FONT_VALIGN_BOTTOM,
					smallFont);

	ProcessUiStack(menuStack);
	DrawUiStack(menuStack);

	if (fadeIn)
	{
		const float alpha = 1.0f - ((float)(state->physicsFrame) / 20.0f);
		Color color = COLOR_BLACK;
		color.a = alpha;
		DrawRect(0, 0, ScaledWindowWidth(), ScaledWindowHeight(), color);

		if (GetState()->physicsFrame >= 20)
		{
			fadeIn = false;
		}
	}
}

void MenuStateSetWithFade()
{
	MenuStateSet();
	fadeIn = true;
}

void MenuStateSet()
{
	GetState()->rpcState = IN_MENUS;
	fadeIn = false;
	if (menuStack == NULL)
	{
		menuStack = CreateUiStack();
		float opY = 80;
		const float opSpacing = 50;

		UiStackPush(menuStack, CreateButtonControl(v2(0, opY), v2(480, 40), "Start", StartGame, MIDDLE_CENTER));
		opY += opSpacing;
		UiStackPush(menuStack, CreateButtonControl(v2(0, opY), v2(480, 40), "Options", OpenOptions, MIDDLE_CENTER));
		opY += opSpacing;
		UiStackPush(menuStack, CreateButtonControl(v2(0, opY), v2(480, 40), "Quit", QuitGame, MIDDLE_CENTER));
		opY += opSpacing;
	}
	UiStackResetFocus(menuStack);
	StopMusic();

	const time_t current = time(NULL);
	const struct tm *t = localtime(&current);
	easterEgg = (t->tm_mon == 3 && t->tm_mday == 1) || HasCliArg("--force-menu-easter-egg");

	SetStateCallbacks(MenuStateUpdate,
					  NULL,
					  GAME_STATE_MENU,
					  MenuStateRender,
					  false); // Fixed update is not needed for this state
}

void MenuStateDestroy()
{
	if (menuStack != NULL)
	{
		DestroyUiStack(menuStack);
		menuStack = NULL;
	}
}
