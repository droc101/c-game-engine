//
// Created by droc101 on 7/29/2024.
//

#include "gameState/LogoSplashState.h"
#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <SDL_gamecontroller.h>
#include <SDL_rect.h>
#include <SDL_scancode.h>
#include <stddef.h>
#include "gameState/MenuState.h"

void LogoSplashStateFixedUpdate(GlobalState *state, double /*delta*/)
{
	if (state->physicsFrame == 20)
	{
		(void)PlaySoundEffect(SOUND("sfx/coincling"), 0, 1, NULL, NULL);
	}

	if (state->physicsFrame == 120 || IsKeyPressed(SDL_SCANCODE_ESCAPE) || IsButtonPressed(SDL_CONTROLLER_BUTTON_START))
	{
		MenuStateSetWithFade();
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void LogoSplashStateRender(GlobalState *State)
{
	if (State->physicsFrame < 20 || State->physicsFrame > 100)
	{
		return;
	}

	const SDL_Rect destRect = {ScaledWindowWidth() / 2 - 150, ScaledWindowHeight() / 2 - 150, 300, 300};

	// HIGH EFFORT FANCY ANIMATION
	float alpha = 1.0f;
	if (State->physicsFrame < 40)
	{
		alpha = (float)(State->physicsFrame - 20) / 20.0f;
	} else if (State->physicsFrame > 80)
	{
		alpha = 1.0f - ((float)(State->physicsFrame - 80) / 20.0f);
	}
	Color color = COLOR_WHITE;
	color.a = alpha;

	DrawTextureMod(v2((float)destRect.x, (float)destRect.y),
				   v2((float)destRect.w, (float)destRect.h),
				   TEXTURE("interface/studio"),
				   &color);
}

void LogoSplashStateSet()
{
	SetStateCallbacks(NULL,
					  LogoSplashStateFixedUpdate,
					  GAME_STATE_LOGO_SPLASH,
					  LogoSplashStateRender,
					  SDL_FALSE); // Non-fixed is not needed for this state
}
