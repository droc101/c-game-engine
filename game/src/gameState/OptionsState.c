//
// Created by droc101 on 10/27/24.
//

#include "gameState/OptionsState.h"
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/CheckBox.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>

#include "gameState/MenuState.h"
#include "gameState/options/DebugOptionsState.h"
#include "gameState/options/InputOptionsState.h"
#include "gameState/options/SoundOptionsState.h"
#include "gameState/options/VideoOptionsState.h"
#include "gameState/PauseState.h"

static UiStack *optionsStack = NULL;
bool optionsStateInGame = false;

static void BtnOptionsBack()
{
	if (optionsStateInGame)
	{
		SetGameState(&PauseState);
	} else
	{
		menuStateFadeIn = false;
		SetGameState(&MenuState);
	}
}

static void OptionsStateUpdate(GlobalState *state, const double delta)
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL))
	{
		BtnOptionsBack();
	}

	if (!optionsStateInGame)
	{
		UpdateMenuBackground(state, delta);
	}
}

void OptionsStateFixedUpdate(GlobalState *state, const double delta)
{
	if (!optionsStateInGame)
	{
		FixedUpdateMenuBackground(state, delta);
	}
}

static void OptionsStateRender(GlobalState *state, const double /*delta*/)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground(state);
	}

	DrawTextAligned("Options",
					32,
					COLOR_WHITE,
					v2s(0),
					v2(ScaledWindowWidthFloat(), 100),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					largeFont);

	ProcessUiStack(optionsStack);
	DrawUiStack(optionsStack);
}

static void BtnVideoOptions()
{
	SetGameState(&VideoOptionsState);
}

static void BtnSoundOptions()
{
	SetGameState(&SoundOptionsState);
}

static void BtnInputOptions()
{
	SetGameState(&InputOptionsState);
}

static void BtnDebugOptions()
{
	SetGameState(&DebugOptionsState);
}

static void CbOptionsEnableDiscordRpc(const bool value)
{
	GetState()->options.enableDiscordRpc = value;
	if (!value)
	{
		DiscordDestroy();
	} else
	{
		DiscordInit();
	}
}

static void OptionsStateSet()
{
	if (optionsStack == NULL)
	{
		optionsStack = CreateUiStack();
		float opY = 80;
		const float opSpacing = 45;

		UiStackPush(optionsStack,
					CreateButtonControl(v2(0, opY), v2(480, 40), "Video Options", BtnVideoOptions, TOP_CENTER));
		opY += opSpacing;
		UiStackPush(optionsStack,
					CreateButtonControl(v2(0, opY), v2(480, 40), "Sound Options", BtnSoundOptions, TOP_CENTER));
		opY += opSpacing;
		UiStackPush(optionsStack,
					CreateButtonControl(v2(0, opY), v2(480, 40), "Input Options", BtnInputOptions, TOP_CENTER));
		opY += opSpacing;
		UiStackPush(optionsStack,
					CreateButtonControl(v2(0, opY), v2(480, 40), "Debug Options", BtnDebugOptions, TOP_CENTER));
#ifdef ENABLE_DISCORD_SDK
		opY += opSpacing * 1.5f;
		UiStackPush(optionsStack,
					CreateCheckboxControl(v2(0, opY),
										  v2(480, 40),
										  "Enable Discord rich presence",
										  CbOptionsEnableDiscordRpc,
										  TOP_CENTER,
										  GetState()->options.enableDiscordRpc));
#endif
		opY += opSpacing;

		UiStackPush(optionsStack, CreateButtonControl(v2(0, -40), v2(480, 40), "Done", BtnOptionsBack, BOTTOM_CENTER));
	}
	UiStackResetFocus(optionsStack);
}

static void OptionsStateDestroy()
{
	if (optionsStack != NULL)
	{
		DestroyUiStack(optionsStack);
		optionsStack = NULL;
	}
}

const GameState OptionsState = {
	.UpdateGame = OptionsStateUpdate,
	.RenderGame = OptionsStateRender,
	.FixedUpdateGame = OptionsStateFixedUpdate,
	.Set = OptionsStateSet,
	.Destroy = OptionsStateDestroy,
	.enableRelativeMouseMode = false,
};
