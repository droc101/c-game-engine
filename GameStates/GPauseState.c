//
// Created by droc101 on 4/22/2024.
//

#include "GPauseState.h"
#include <SDL_gamecontroller.h>
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Core/SoundSystem.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/GlobalState.h"
#include "../Structs/UI/Controls/Button.h"
#include "../Structs/UI/UiStack.h"
#include "../Structs/Vector2.h"
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
		(void)PlaySoundEffect(SOUND("sfx/popdown"), 0, 1, NULL, NULL);
		GMainStateSet();
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GPauseStateRender(GlobalState * /*state*/)
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
