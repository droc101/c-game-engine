//
// Created by droc101 on 11/23/2024.
//

#include "GVideoOptionsState.h"
#include <stdio.h>
#include "../../Helpers/CommonAssets.h"
#include "../../Helpers/Core/Error.h"
#include "../../Helpers/Core/Input.h"
#include "../../Helpers/Graphics/Drawing.h"
#include "../../Helpers/Graphics/Font.h"
#include "../../Helpers/Graphics/RenderingHelpers.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Options.h"
#include "../../Structs/UI/Controls/Button.h"
#include "../../Structs/UI/Controls/CheckBox.h"
#include "../../Structs/UI/Controls/RadioButton.h"
#include "../../Structs/UI/Controls/Slider.h"
#include "../../Structs/UI/UiStack.h"
#include "../../Structs/Vector2.h"
#include "../GOptionsState.h"

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
	const SliderData *data = (SliderData *)slider->ControlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %s", data->label, labels[(int)data->value]);
	return buf;
}

char *SliderLabelMipmaps(const Control *slider)
{
	char *labels[] = {"None", "1", "2", "3", "4"};
	const SliderData *data = (SliderData *)slider->ControlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %s", data->label, labels[(int)data->value]);
	return buf;
}

char *SliderLabelLod(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->ControlData;
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

void RbOptionsRenderer(const bool /*value*/, const byte /*groupId*/, const byte id)
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

void CbOptionsPreferWayland(const bool value)
{
	GetState()->options.preferWayland = value;
	hasChangedVideoOptions = true;
	// Change will happen next restart
}

void SldOptionsMsaa(const double value)
{
	GetState()->options.msaa = value;
	hasChangedVideoOptions = true;
	// Change will happen next restart
}

void SldOptionsMipmaps(const double value)
{
	GetState()->options.mipmaps = value;
	hasChangedVideoOptions = true;
}

void SldOptionsLod(const double value)
{
	GetState()->options.lodMultiplier = value;
}

void GVideoOptionsStateUpdate(GlobalState * /*State*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(CONTROLLER_CANCEL))
	{
		BtnVideoOptionsBack();
	}
}

void GVideoOptionsStateRender(GlobalState *)
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
		opY += opSpacing * 1.5f;

		UiStackPush(videoOptionsStack,
					CreateRadioButtonControl(v2(0, opY),
											 v2(480, 40),
											 "Vulkan (WIP)",
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
										"Mipmaps",
										SldOptionsMipmaps,
										TOP_CENTER,
										0.0,
										4.0,
										GetState()->options.mipmaps,
										1,
										1,
										SliderLabelMipmaps));

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
					  VIDEO_OPTIONS_STATE,
					  GVideoOptionsStateRender); // Fixed update is not needed for this state
}

void GVideoOptionsStateDestroy()
{
	if (videoOptionsStack != NULL)
	{
		DestroyUiStack(videoOptionsStack);
		videoOptionsStack = NULL;
	}
}
