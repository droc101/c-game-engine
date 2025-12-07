//
// Created by droc101 on 11/23/2024.
//

#include "gameState/options/SoundOptionsState.h"
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/Slider.h>
#include <engine/uiStack/UiStack.h>
#include <SDL_scancode.h>
#include <SDL_stdinc.h>
#include <stddef.h>
#include <stdint.h>
#include "gameState/OptionsState.h"

UiStack *soundOptionsStack = NULL;

void BtnSoundOptionsBack()
{
	OptionsStateSet(optionsStateInGame);
}

void SldOptionsMasterVolume(const float value)
{
	GetState()->options.masterVolume = value;
	UpdateVolume();
}

void SldOptionsMusicVolume(const float value)
{
	GetState()->options.musicVolume = value;
	UpdateVolume();
}

void SldOptionsSfxVolume(const float value)
{
	GetState()->options.sfxVolume = value;
	UpdateVolume();
}

void SoundOptionsStateUpdate(GlobalState * /*state*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(CONTROLLER_CANCEL))
	{
		BtnSoundOptionsBack();
	}
}

void SoundOptionsStateRender(GlobalState * /*state*/)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground();
	}

	DrawTextAligned("Sound Options",
					32,
					COLOR_WHITE,
					v2s(0),
					v2(ScaledWindowWidthFloat(), 100),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					largeFont);

	ProcessUiStack(soundOptionsStack);
	DrawUiStack(soundOptionsStack);
}

void SoundOptionsStateSet()
{
	if (soundOptionsStack == NULL)
	{
		soundOptionsStack = CreateUiStack();
		uint16_t opY = 80;
		const uint16_t opSpacing = 45;
		UiStackPush(soundOptionsStack,
					CreateSliderControl(v2(0, opY),
										v2(480, 40),
										"Master Volume",
										SldOptionsMasterVolume,
										TOP_CENTER,
										0.0,
										1.0,
										GetState()->options.masterVolume,
										0.01,
										0.1,
										SliderLabelPercent));
		opY += opSpacing;
		UiStackPush(soundOptionsStack,
					CreateSliderControl(v2(0, opY),
										v2(480, 40),
										"Music Volume",
										SldOptionsMusicVolume,
										TOP_CENTER,
										0.0,
										1.0,
										GetState()->options.musicVolume,
										0.01,
										0.1,
										SliderLabelPercent));
		opY += opSpacing;
		UiStackPush(soundOptionsStack,
					CreateSliderControl(v2(0, opY),
										v2(480, 40),
										"SFX Volume",
										SldOptionsSfxVolume,
										TOP_CENTER,
										0.0,
										1.0,
										GetState()->options.sfxVolume,
										0.01,
										0.1,
										SliderLabelPercent));
		opY += opSpacing;


		UiStackPush(soundOptionsStack,
					CreateButtonControl(v2(0, -40), v2(480, 40), "Back", BtnSoundOptionsBack, BOTTOM_CENTER));
	}
	UiStackResetFocus(soundOptionsStack);

	SetStateCallbacks(SoundOptionsStateUpdate,
					  NULL,
					  GAME_STATE_SOUND_OPTIONS,
					  SoundOptionsStateRender,
					  SDL_FALSE); // Fixed update is not needed for this state
}

void SoundOptionsStateDestroy()
{
	if (soundOptionsStack != NULL)
	{
		DestroyUiStack(soundOptionsStack);
		soundOptionsStack = NULL;
	}
}
