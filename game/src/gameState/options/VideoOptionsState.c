//
// Created by droc101 on 11/23/2024.
//

#include "gameState/options/VideoOptionsState.h"
#include <SDL_scancode.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Options.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/CheckBox.h>
#include <engine/uiStack/controls/RadioButton.h>
#include <engine/uiStack/controls/Slider.h>
#include <engine/uiStack/UiStack.h>
#include <engine/structs/Vector2.h>
#include "gameState/OptionsState.h"

UiStack *videoOptionsStack = NULL;
bool hasChangedVideoOptions = false;

void BtnVideoOptionsBack()
{
	if (hasChangedVideoOptions)
	{
		SaveOptions(&GetState()->options);
		PromptRelaunch("Restart Game?",
					   "You have changed options that require a relaunch. Would you like to relaunch now?",
					   "Yes",
					   "No");
	}
	GOptionsStateSet(optionsStateInGame);
}

char *SliderLabelMSAA(const Control *slider)
{
	char *labels[] = {"Off", "2X", "4X", "8X"};
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %s", data->label, labels[(int)data->value]);
	return buf;
}

char *SliderLabelLod(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %.1fx", data->label, data->value);
	return buf;
}

void CbOptionsFullscreen(const bool value)
{
	GetState()->options.fullscreen = value;
	SDL_SetWindowFullscreen(GetGameWindow(), value ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void RbOptionsRenderer(const bool /*value*/, const uint8_t /*groupId*/, const uint8_t id)
{
	GetState()->options.renderer = id;
	hasChangedVideoOptions = true;
	// Renderer change will happen on next restart
}

void CbOptionsVsync(const bool value)
{
	GetState()->options.vsync = value;
	hasChangedVideoOptions = true;
	// VSync change will happen on next restart
}

void CbOptionsLimitFpsWhenUnfocused(const bool value)
{
	GetState()->options.limitFpsWhenUnfocused = value;
}

void CbOptionsMipmaps(const bool value)
{
	GetState()->options.mipmaps = value;
	hasChangedVideoOptions = true;
	// Mipmaps change will happen on next restart
}

void CbOptionsPreferWayland(const bool value)
{
	GetState()->options.preferWayland = value;
	hasChangedVideoOptions = true;
	// Change will happen next restart
}

void SldOptionsMsaa(const float value)
{
	GetState()->options.msaa = value;
	hasChangedVideoOptions = true;
	// Change will happen next restart
}

void SldOptionsLod(const float value)
{
	GetState()->options.lodMultiplier = value;
}

void SldOptionsFov(const float value)
{
	GetState()->options.fov = value;
	GetState()->camera->fov = GetState()->options.fov;
}

void GVideoOptionsStateUpdate(GlobalState * /*State*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(CONTROLLER_CANCEL))
	{
		BtnVideoOptionsBack();
	}
}

void GVideoOptionsStateRender(GlobalState * /*state*/)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground();
	}

	DrawTextAligned("Video Options",
					32,
					COLOR_WHITE,
					v2s(0),
					v2(WindowWidthFloat(), 100),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					largeFont);

	ProcessUiStack(videoOptionsStack);
	DrawUiStack(videoOptionsStack);
}

void GVideoOptionsStateSet()
{
	if (videoOptionsStack == NULL)
	{
		videoOptionsStack = CreateUiStack();
		float opY = 80;
		const float opSpacing = 45;
		UiStackPush(videoOptionsStack,
					CreateSliderControl(v2(0, opY),
										v2(480, 40),
										"FOV",
										SldOptionsFov,
										TOP_CENTER,
										30.0,
										120.0,
										GetState()->options.fov,
										1,
										1,
										NULL));

		opY += opSpacing * 1.5f;
		UiStackPush(videoOptionsStack,
					CreateCheckboxControl(v2(0, opY),
										  v2(480, 40),
										  "Fullscreen",
										  CbOptionsFullscreen,
										  TOP_CENTER,
										  GetState()->options.fullscreen));
		opY += opSpacing;
		UiStackPush(videoOptionsStack,
					CreateCheckboxControl(v2(0, opY),
										  v2(480, 40),
										  "VSync",
										  CbOptionsVsync,
										  TOP_CENTER,
										  GetState()->options.vsync));
		opY += opSpacing;
		UiStackPush(videoOptionsStack,
					CreateCheckboxControl(v2(0, opY),
										  v2(480, 40),
										  "Limit FPS when in background",
										  CbOptionsLimitFpsWhenUnfocused,
										  TOP_CENTER,
										  GetState()->options.limitFpsWhenUnfocused));
		opY += opSpacing;
		UiStackPush(videoOptionsStack,
					CreateCheckboxControl(v2(0, opY),
										  v2(480, 40),
										  "Mipmaps",
										  CbOptionsMipmaps,
										  TOP_CENTER,
										  GetState()->options.mipmaps));
		opY += opSpacing * 1.5f;

		UiStackPush(videoOptionsStack,
					CreateRadioButtonControl(v2(0, opY),
											 v2(480, 40),
											 "Vulkan",
											 RbOptionsRenderer,
											 TOP_CENTER,
											 GetState()->options.renderer == RENDERER_VULKAN,
											 0,
											 RENDERER_VULKAN));

		opY += opSpacing;
		UiStackPush(videoOptionsStack,
					CreateRadioButtonControl(v2(0, opY),
											 v2(480, 40),
											 "OpenGL (Compatibility)",
											 RbOptionsRenderer,
											 TOP_CENTER,
											 GetState()->options.renderer == RENDERER_OPENGL,
											 0,
											 RENDERER_OPENGL));
		opY += opSpacing * 1.5f;
		UiStackPush(videoOptionsStack,
					CreateSliderControl(v2(0, opY),
										v2(480, 40),
										"MSAA",
										SldOptionsMsaa,
										TOP_CENTER,
										0.0,
										3.0,
										GetState()->options.msaa,
										1,
										1,
										SliderLabelMSAA));

		opY += opSpacing;
		UiStackPush(videoOptionsStack,
					CreateSliderControl(v2(0, opY),
										v2(480, 40),
										"LOD Distance",
										SldOptionsLod,
										TOP_CENTER,
										0.5,
										2.0,
										GetState()->options.lodMultiplier,
										0.5,
										1,
										SliderLabelLod));
#ifdef __LINUX__
		opY += opSpacing * 1.5f;
		UiStackPush(videoOptionsStack,
					CreateCheckboxControl(v2(0, opY),
										  v2(480, 40),
										  "Prefer Wayland over X11",
										  CbOptionsPreferWayland,
										  TOP_CENTER,
										  GetState()->options.preferWayland));
#endif
		opY += opSpacing;


		UiStackPush(videoOptionsStack,
					CreateButtonControl(v2(0, -40), v2(480, 40), "Back", BtnVideoOptionsBack, BOTTOM_CENTER));
	}
	UiStackResetFocus(videoOptionsStack);
	hasChangedVideoOptions = false;

	SetStateCallbacks(GVideoOptionsStateUpdate,
					  NULL,
					  GAME_STATE_VIDEO_OPTIONS,
					  GVideoOptionsStateRender,
					  SDL_FALSE); // Fixed update is not needed for this state
}

void GVideoOptionsStateDestroy()
{
	if (videoOptionsStack != NULL)
	{
		DestroyUiStack(videoOptionsStack);
		videoOptionsStack = NULL;
	}
}
