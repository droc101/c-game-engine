//
// Created by droc101 on 4/22/2024.
//

#include "GMenuState.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "../config.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/SoundSystem.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/Color.h"
#include "../Structs/GlobalState.h"
#include "../Structs/UI/Controls/Button.h"
#include "../Structs/UI/UiStack.h"
#include "../Structs/Vector2.h"
#include "GLevelSelectState.h"
#include "GOptionsState.h"

#ifndef USE_LEVEL_SELECT
#include "GMainState.h"
#endif

UiStack *menuStack = NULL;
bool fadeIn = false;

void StartGame()
{
#ifdef USE_LEVEL_SELECT
	GLevelSelectStateSet();
#else
	GMainStateSet();
#endif
}

void QuitGame()
{
	GetState()->requestExit = true;
}

void OpenOptions()
{
	GOptionsStateSet(false);
}

void GMenuStateUpdate(GlobalState * /*State*/) {}

void GMenuStateRender(GlobalState *State)
{
	RenderMenuBackground();

	// draw the logo
	Vector2 logoPosition;
	Vector2 logoSize;
	logoPosition.x = ((float)WindowWidth() - 480) / 2;
	logoPosition.y = 32;
	logoSize.x = 480;
	logoSize.y = 320;
	DrawTexture(logoPosition, logoSize, TEXTURE("interface/menu_logo"));

#ifdef BUILDSTYLE_DEBUG
	FontDrawString(v2(20, 200), "DEBUG BUILD", 16, COLOR(0xFF00FF00), smallFont);
#endif

	// draw version and copyright info
	char buffer[256];
	sprintf(buffer, "Engine %s\n%s", VERSION, COPYRIGHT);
	DrawTextAligned(buffer,
					16,
					COLOR_BLACK,
					v2(WindowWidthFloat() - 208, WindowHeightFloat() - 208),
					v2(200, 200),
					FONT_HALIGN_RIGHT,
					FONT_VALIGN_BOTTOM,
					smallFont);
	DrawTextAligned(buffer,
					16,
					COLOR(0xFFa0a0a0),
					v2(WindowWidthFloat() - 210, WindowHeightFloat() - 210),
					v2(200, 200),
					FONT_HALIGN_RIGHT,
					FONT_VALIGN_BOTTOM,
					smallFont);

	ProcessUiStack(menuStack);
	DrawUiStack(menuStack);

	if (fadeIn)
	{
		const float alpha = 1.0f - ((float)(State->physicsFrame) / 20.0f);
		Color color = COLOR_BLACK;
		color.a = alpha;
		DrawRect(0, 0, WindowWidth(), WindowHeight(), color);

		if (GetState()->physicsFrame >= 20)
		{
			fadeIn = false;
		}
	}
}

void GMenuStateSetWithFade()
{
	GMenuStateSet();
	fadeIn = true;
}

void GMenuStateSet()
{
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

	SetStateCallbacks(GMenuStateUpdate,
					  NULL,
					  MENU_STATE,
					  GMenuStateRender); // Fixed update is not needed for this state
}

void GMenuStateDestroy()
{
	if (menuStack != NULL)
	{
		DestroyUiStack(menuStack);
		menuStack = NULL;
	}
}
