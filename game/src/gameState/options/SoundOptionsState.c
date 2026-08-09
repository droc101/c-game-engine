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
#include "helpers/OptionsMenu.h"

static OptionsMenu *soundOptionsMenu = NULL;

static void BtnSoundOptionsBack(Control *, void *)
{
	SaveOptions(&GetState()->options);
	SetGameState(&OptionsState);
}

static void SldOptionsVolume(const ControlValue * /*value*/)
{
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

	ProcessOptionsMenu(soundOptionsMenu);
}

static void SoundOptionsStateSet()
{
	if (soundOptionsMenu == NULL)
	{
		soundOptionsMenu = CreateOptionsMenu();
		OptionsMenuAddLargeControl(soundOptionsMenu,
								   CreateSliderControl(v2s(0),
													   v2s(0),
													   "Master Volume",
													   SldOptionsVolume,
													   TOP_CENTER,
													   0.0f,
													   1.0f,
													   (ControlValue){
														   .type = CONTROL_VALUE_FLOAT,
														   .floatValue = &GetState()->options.masterVolume,
													   },
													   0.01f,
													   0.1f,
													   SliderLabelPercent,
													   NULL));
		OptionsMenuAddSection(soundOptionsMenu, NULL);
		OptionsMenuAddSmallControl(soundOptionsMenu,
								   CreateSliderControl(v2s(0),
													   v2s(0),
													   "Music Volume",
													   SldOptionsVolume,
													   TOP_CENTER,
													   0.0f,
													   1.0f,
													   (ControlValue){
														   .type = CONTROL_VALUE_FLOAT,
														   .floatValue = &GetState()->options.musicVolume,
													   },
													   0.01f,
													   0.1f,
													   SliderLabelPercent,
													   NULL));
		OptionsMenuAddSmallControl(soundOptionsMenu,
								   CreateSliderControl(v2s(0),
													   v2s(0),
													   "SFX Volume",
													   SldOptionsVolume,
													   TOP_CENTER,
													   0.0f,
													   1.0f,
													   (ControlValue){
														   .type = CONTROL_VALUE_FLOAT,
														   .floatValue = &GetState()->options.sfxVolume,
													   },
													   0.01f,
													   0.1f,
													   SliderLabelPercent,
													   NULL));
		OptionsMenuAddSmallControl(soundOptionsMenu,
								   CreateSliderControl(v2s(0),
													   v2s(0),
													   "UI Volume",
													   SldOptionsVolume,
													   TOP_CENTER,
													   0.0f,
													   1.0f,
													   (ControlValue){
														   .type = CONTROL_VALUE_FLOAT,
														   .floatValue = &GetState()->options.uiVolume,
													   },
													   0.01f,
													   0.1f,
													   SliderLabelPercent,
													   NULL));

		OptionsMenuAddSimpleHeaderFooter(soundOptionsMenu, "Sound Options", BtnSoundOptionsBack);
	}
	UiStackResetFocus(soundOptionsMenu->stack);
}

static void SoundOptionsStateDestroy()
{
	if (soundOptionsMenu != NULL)
	{
		DestroyOptionsMenu(soundOptionsMenu);
		soundOptionsMenu = NULL;
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
