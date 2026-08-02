//
// Created by droc101 on 11/23/2024.
//

#include "gameState/options/SoundOptionsState.h"
#include <engine/graphics/Drawing.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Options.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/HeaderFooterControl.h>
#include <engine/uiStack/controls/Slider.h>
#include <engine/uiStack/ScrollView.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "gameState/OptionsState.h"

static UiStack *soundOptionsStack = NULL;
static ScrollView *soundOptionsScrollView = NULL;

static void BtnSoundOptionsBack(Control *, void *)
{
	SaveOptions(&GetState()->options);
	SetGameState(&OptionsState);
}

static void SldOptionsMasterVolume(const float value)
{
	GetState()->options.masterVolume = value;
	UpdateVolume();
}

static void SldOptionsMusicVolume(const float value)
{
	GetState()->options.musicVolume = value;
	UpdateVolume();
}

static void SldOptionsSfxVolume(const float value)
{
	GetState()->options.sfxVolume = value;
	UpdateVolume();
}

static void SldOptionsUiVolume(const float value)
{
	GetState()->options.uiVolume = value;
	UpdateVolume();
}

static void SoundOptionsStateUpdate(GlobalState *state, const double delta)
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL))
	{
		BtnSoundOptionsBack(NULL, NULL);
	}
	if (!optionsStateInGame)
	{
		UpdateMenuBackground(state, delta);
	}
}

static void SoundOptionsStateRender(GlobalState *state, const double /*delta*/)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground(state);
	}

	soundOptionsScrollView->size.y = ScaledWindowHeightFloat() - 200;
	ProcessScrollView(soundOptionsScrollView);
	ProcessUiStack(soundOptionsStack);
	DrawUiStack(soundOptionsStack);
}

static void SoundOptionsStateSet()
{
	if (soundOptionsStack == NULL)
	{
		soundOptionsStack = CreateUiStack();
		soundOptionsScrollView = CreateScrollView(soundOptionsStack, TOP_CENTER, v2(0, 100), v2(750, 200));
		uint16_t opY = 0;
		const uint16_t opSpacing = 45;
		ScrollViewAddChild(soundOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
											   v2(750, 40),
											   "Master Volume",
											   SldOptionsMasterVolume,
											   TOP_CENTER,
											   0.0,
											   1.0,
											   GetState()->options.masterVolume,
											   0.01,
											   0.1,
											   SliderLabelPercent,
											   NULL));
		opY += opSpacing * 1.5;
		ScrollViewAddChild(soundOptionsScrollView,
						   CreateSliderControl(v2(-190, opY),
											   v2(370, 40),
											   "Music Volume",
											   SldOptionsMusicVolume,
											   TOP_CENTER,
											   0.0,
											   1.0,
											   GetState()->options.musicVolume,
											   0.01,
											   0.1,
											   SliderLabelPercent,
											   NULL));
		ScrollViewAddChild(soundOptionsScrollView,
						   CreateSliderControl(v2(190, opY),
											   v2(370, 40),
											   "SFX Volume",
											   SldOptionsSfxVolume,
											   TOP_CENTER,
											   0.0,
											   1.0,
											   GetState()->options.sfxVolume,
											   0.01,
											   0.1,
											   SliderLabelPercent,
											   NULL));
		opY += opSpacing;
		ScrollViewAddChild(soundOptionsScrollView,
						   CreateSliderControl(v2(-190, opY),
											   v2(370, 40),
											   "UI Volume",
											   SldOptionsUiVolume,
											   TOP_CENTER,
											   0.0,
											   1.0,
											   GetState()->options.uiVolume,
											   0.01,
											   0.1,
											   SliderLabelPercent,
											   NULL));
		opY += opSpacing;


		UiStackPush(soundOptionsStack, CreateHeaderFooterControl(100, true, "Sound Options"));
		UiStackPush(soundOptionsStack, CreateHeaderFooterControl(100, false, NULL));
		UiStackPush(soundOptionsStack,
					CreateButtonControl(v2(0, -40), v2(480, 40), "Back", BtnSoundOptionsBack, BOTTOM_CENTER, NULL));
	}
	UiStackResetFocus(soundOptionsStack);
}

static void SoundOptionsStateDestroy()
{
	if (soundOptionsStack != NULL)
	{
		FreeScrollView(soundOptionsScrollView);
		DestroyUiStack(soundOptionsStack);
		soundOptionsStack = NULL;
	}
}

const GameState SoundOptionsState = {
	.UpdateGame = SoundOptionsStateUpdate,
	.RenderGame = SoundOptionsStateRender,
	.FixedUpdateGame = OptionsStateFixedUpdate,
	.Destroy = SoundOptionsStateDestroy,
	.Set = SoundOptionsStateSet,
	.enableRelativeMouseMode = false,
};
