//
// Created by droc101 on 7/29/2024.
//

#include "GLogoSplashState.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "GMenuState.h"

void GLogoSplashStateFixedUpdate(GlobalState *State, double /*delta*/)
{
#ifdef DEBUG_NOSPLASH
	if (State->physicsFrame == 1)
	{
		GMenuStateSet();
	}
	if (State->physicsFrame > 0)
	{
		State->physicsFrame++;
		return;
	}
#endif

	if (State->physicsFrame == 20)
	{
		PlaySoundEffect(SOUND("sfx_coincling"));
	}

	if (State->physicsFrame == 120 || IsKeyPressed(SDL_SCANCODE_ESCAPE) || IsButtonPressed(SDL_CONTROLLER_BUTTON_START))
	{
		GMenuStateSet();
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GLogoSplashStateRender(GlobalState *State)
{
	if (State->physicsFrame < 20 || State->physicsFrame > 100)
	{
		return;
	}

	const SDL_Rect destRect = {WindowWidth() / 2 - 150, WindowHeight() / 2 - 150, 300, 300};

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
				   TEXTURE("interface_studio"),
				   color);
}

void GLogoSplashStateSet()
{
	SetStateCallbacks(NULL,
					  GLogoSplashStateFixedUpdate,
					  LOGO_SPLASH_STATE,
					  GLogoSplashStateRender); // Non-fixed is not needed for this state
}
