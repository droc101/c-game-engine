//
// Created by droc101 on 11/23/2024.
//

#include "gameState/options/VideoOptionsState.h"
#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Options.h>
#include <engine/structs/Vector2.h>
#include <engine/structs/VideoPreset.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/HeaderFooterControl.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/controls/OptionsButton.h>
#include <engine/uiStack/controls/Slider.h>
#include <engine/uiStack/ScrollView.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "gameState/options/RestartPromptState.h"
#include "gameState/OptionsState.h"

static UiStack *videoOptionsStack = NULL;
static ScrollView *videoOptionsScrollView = NULL;
static bool hasChangedVideoOptions = false;

VideoPreset currentVideoPreset;

static OptionsButtonValue gpuTypeButtonValues[3] = {
	{
		.text = "Dedicated",
		.tooltip = "Prefer using a dedicated GPU, which usually provides the best performance.",
		.value = {.type = CONTROL_VALUE_DWORD, .dwordValue = GPU_TYPE_DEDICATED},
	},
	{
		.text = "Integrated",
		.tooltip = "Prefer using an integrated GPU, which usually provides less performance than a dedicated GPU.",
		.value = {.type = CONTROL_VALUE_DWORD, .dwordValue = GPU_TYPE_INTEGRATED},
	},
	{
		.text = "Software",
		.tooltip = "Prefer using a software emulated GPU, which usually provides the worst performance.",
		.value = {.type = CONTROL_VALUE_DWORD, .dwordValue = GPU_TYPE_SOFTWARE},
	},
};

static OptionsButtonValue preferWaylandButtonValues[2] = {
	{
		.text = "Prefer X11",
		.tooltip = NULL,
		.value =
				{
					.type = CONTROL_VALUE_BOOL,
					.boolValue = false,
				},
	},
	{
		.text = "Prefer Wayland",
		.tooltip = NULL,
		.value =
				{
					.type = CONTROL_VALUE_BOOL,
					.boolValue = true,
				},
	},
};

static void BtnVideoOptionsBack(Control *, void *)
{
	SaveOptions(&GetState()->options);
	if (hasChangedVideoOptions)
	{
		SetGameState(&RestartPromptState);
		return;
	}
	SetGameState(&OptionsState);
}

static char *SliderLabelShadowMapQuality(const Control *slider)
{
	char *labels[] = {"Disabled", "128px", "256px", "512px", "1024px", "2048px", "4096px", "8192px", "16384px"};
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %s", data->label, labels[(int)GetSliderValueAsFloat(slider->controlData)]);
	return buf;
}

static char *SliderLabelMSAA(const Control *slider)
{
	char *labels[] = {"Off", "2x MSAA", "4x MSAA", "8x MSAA"};
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %s", data->label, labels[(int)GetSliderValueAsFloat(slider->controlData)]);
	return buf;
}

static char *SliderLabelAnisotropy(const Control *slider)
{
	char *labels[] = {"Off", "2x", "4x", "8x", "16x"};
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %s", data->label, labels[(int)GetSliderValueAsFloat(slider->controlData)]);
	return buf;
}

static char *SliderLabelVideoPreset(const Control *slider)
{
	char *labels[] = {"Very Low", "Low", "Medium", "High", "Ultra", "Custom"};
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %s", data->label, labels[(int)GetSliderValueAsFloat(slider->controlData)]);
	return buf;
}

static char *SliderLabelMaxFps(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	float value = GetSliderValueAsFloat(slider->controlData);
	if (value == 0)
	{
		sprintf(buf, "%s: Unlimited", data->label);
	} else
	{
		sprintf(buf, "%s: %.0f", data->label, value);
	}
	return buf;
}

static void OptBtnFullscreenChanged(const OptionsButtonValue *value, void *)
{
	SDL_SetWindowFullscreen(GetGameWindow(), GetState()->options.fullscreen);
}

static void ToggleVsyncCallback(const OptionsButtonValue *value, void *)
{
	hasChangedVideoOptions = true; // Until Luna can do this
	rendererQueuedActions |= QUEUED_ACTION_TOGGLE_VSYNC;
}

static void ClearTexturesOptBtnCallback(const OptionsButtonValue *value, void * /*extraData*/)
{
	rendererQueuedActions |= QUEUED_ACTION_CLEAR_ALL_TEXTURES;
}

static void ClearTexturesSliderCallback(const ControlValue * /*value*/)
{
	rendererQueuedActions |= QUEUED_ACTION_CLEAR_ALL_TEXTURES;
}

static void RequireRestartCallback(const OptionsButtonValue *value, void *)
{
	hasChangedVideoOptions = true;
	// Change will happen next restart
}

static void SldOptionsShadowMapQuality(const ControlValue * /*value*/)
{
	rendererQueuedActions |= QUEUED_ACTION_UPDATE_SHADOW_MAP_RESOLUTION;
}

static void UpdateMsaaCallback(const ControlValue * /*value*/)
{
	hasChangedVideoOptions = true; // Until Luna can do this
	rendererQueuedActions |= QUEUED_ACTION_UPDATE_MSAA;
}

static void UpdateVideoPresetCallback(const ControlValue * /*value*/)
{
	ApplyVideoPreset(&GetState()->options, currentVideoPreset);
	rendererQueuedActions |= QUEUED_ACTION_CLEAR_ALL_TEXTURES |
							 QUEUED_ACTION_UPDATE_MSAA |
							 QUEUED_ACTION_UPDATE_SHADOW_MAP_RESOLUTION;
	hasChangedVideoOptions = true; // until luna can change msaa
}

static void UpdateFovCallback(const ControlValue * /*value*/)
{
	if (GetState()->map)
	{
		GetState()->map->player.playerCamera.fov = GetState()->options.fov;
	}
}

static void VideoOptionsStateUpdate(GlobalState *state, const double delta)
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL))
	{
		BtnVideoOptionsBack(NULL, NULL);
	}
	if (!optionsStateInGame)
	{
		UpdateMenuBackground(state, delta);
	}

	currentVideoPreset = GetCurrentVideoPreset(&state->options);
}

static void VideoOptionsStateRender(GlobalState *state, const double /*delta*/)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground(state);
	}

	videoOptionsScrollView->size.y = ScaledWindowHeightFloat() - 200;
	ProcessScrollView(videoOptionsScrollView);
	ProcessUiStack(videoOptionsStack);
	DrawUiStack(videoOptionsStack);
}

static void VideoOptionsStateSet()
{
	if (videoOptionsStack == NULL)
	{
		videoOptionsStack = CreateUiStack();
		videoOptionsScrollView = CreateScrollView(videoOptionsStack, TOP_CENTER, v2(0, 100), v2(750, 200));
		float opY = 0;
		const float opSpacing = 45;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
											   v2(750, 40),
											   "FOV",
											   UpdateFovCallback,
											   TOP_CENTER,
											   30.0f,
											   120.0f,
											   (ControlValue){
												   .type = CONTROL_VALUE_FLOAT,
												   .floatValue = &GetState()->options.fov,
											   },
											   1.0f,
											   1.0f,
											   NULL,
											   NULL));

		opY += opSpacing * 1.5f;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateLabelControl("Display Options",
											  16,
											  COLOR_WHITE,
											  v2(0, opY),
											  v2(750, 40),
											  TOP_CENTER,
											  FONT_HALIGN_LEFT,
											  FONT_VALIGN_MIDDLE,
											  FONT("small_font"),
											  true));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
											   v2(750, 40),
											   "Maximum FPS",
											   NULL,
											   TOP_CENTER,
											   0.0f,
											   500.0f,
											   (ControlValue){
												   .type = CONTROL_VALUE_WORD,
												   .wordValue = &GetState()->options.maxFps,
											   },
											   10.0f,
											   10.0f,
											   SliderLabelMaxFps,
											   NULL));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateOptionsButtonControl(v2(-190, opY),
													  v2(370, 40),
													  "VSync: %s",
													  ToggleVsyncCallback,
													  TOP_CENTER,
													  onOffButtonValues,
													  2,
													  NULL,
													  (ControlValue){
														  .type = CONTROL_VALUE_BOOL,
														  .boolValue = &GetState()->options.vsync,
													  },
													  "Limits the framerate to match your monitor's refresh rate to "
													  "reduce screen tearing"));
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateOptionsButtonControl(v2(190, opY),
													  v2(370, 40),
													  "Limit Background FPS: %s",
													  NULL,
													  TOP_CENTER,
													  yesNoButtonValues,
													  2,
													  NULL,
													  (ControlValue){
														  .type = CONTROL_VALUE_BOOL,
														  .boolValue = &GetState()->options.limitFpsWhenUnfocused,
													  },
													  "Limit the framerate to 30 when the game window is not focused"));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateOptionsButtonControl(v2(-190, opY),
													  v2(370, 40),
													  "Fullscreen: %s",
													  OptBtnFullscreenChanged,
													  TOP_CENTER,
													  onOffButtonValues,
													  2,
													  NULL,
													  (ControlValue){
														  .type = CONTROL_VALUE_BOOL,
														  .boolValue = &GetState()->options.fullscreen,
													  },
													  NULL));
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateOptionsButtonControl(v2(190, opY),
													  v2(370, 40),
													  "Preferred GPU Type: %s",
													  RequireRestartCallback,
													  TOP_CENTER,
													  gpuTypeButtonValues,
													  3,
													  NULL,
													  (ControlValue){
														  .type = CONTROL_VALUE_DWORD,
														  .dwordValue = &GetState()->options.preferredGpuType,
													  },
													  NULL));
#ifdef SDL_PLATFORM_LINUX
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateOptionsButtonControl(v2(-190, opY),
													  v2(370, 40),
													  "Video Platform: %s",
													  RequireRestartCallback,
													  TOP_CENTER,
													  preferWaylandButtonValues,
													  2,
													  NULL,
													  (ControlValue){
														  .type = CONTROL_VALUE_BOOL,
														  .boolValue = &GetState()->options.preferWayland,
													  },
													  NULL));
#endif
		opY += opSpacing * 1.5f;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateLabelControl("Quality Options",
											  16,
											  COLOR_WHITE,
											  v2(0, opY),
											  v2(750, 40),
											  TOP_CENTER,
											  FONT_HALIGN_LEFT,
											  FONT_VALIGN_MIDDLE,
											  FONT("small_font"),
											  true));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
											   v2(750, 40),
											   "Preset",
											   UpdateVideoPresetCallback,
											   TOP_CENTER,
											   0.0f,
											   VIDEO_PRESET_CUSTOM,
											   (ControlValue){
												   .type = CONTROL_VALUE_DWORD,
												   .dwordValue = &currentVideoPreset,
											   },
											   1.0f,
											   1.0f,
											   SliderLabelVideoPreset,
											   NULL));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
											   v2(750, 40),
											   "Shadow Resolution",
											   SldOptionsShadowMapQuality,
											   TOP_CENTER,
											   0.0,
											   8.0,
											   (ControlValue){
												   .type = CONTROL_VALUE_DWORD,
												   .dwordValue = &GetState()->options.shadowMapQuality,
											   },
											   1,
											   1,
											   SliderLabelShadowMapQuality,
											   "The resolution to use for shadow maps"));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
											   v2(750, 40),
											   "Anti-Aliasing",
											   UpdateMsaaCallback,
											   TOP_CENTER,
											   0.0f,
											   3.0f,
											   (ControlValue){
												   .type = CONTROL_VALUE_DWORD,
												   .dwordValue = &GetState()->options.msaa,
											   },
											   1.0f,
											   1.0f,
											   SliderLabelMSAA,
											   "Smooths the edges of objects"));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
											   v2(750, 40),
											   "LOD Distance",
											   NULL,
											   TOP_CENTER,
											   0.5f,
											   2.0f,
											   (ControlValue){
												   .type = CONTROL_VALUE_FLOAT,
												   .floatValue = &GetState()->options.lodMultiplier,
											   },
											   0.5f,
											   1.0f,
											   SliderLabelPercent,
											   "Changes the level of detail on far away objects"));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateOptionsButtonControl(v2(-190, opY),
													  v2(370, 40),
													  "Mipmaps: %s",
													  ClearTexturesOptBtnCallback,
													  TOP_CENTER,
													  onOffButtonValues,
													  2,
													  NULL,
													  (ControlValue){
														  .type = CONTROL_VALUE_BOOL,
														  .boolValue = &GetState()->options.mipmaps,
													  },
													  "Improves the appearance of far away textures"));
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateSliderControl(v2(190, opY),
											   v2(370, 40),
											   "Anisotropic Filtering",
											   ClearTexturesSliderCallback,
											   TOP_CENTER,
											   0.0f,
											   4.0f,
											   (ControlValue){
												   .type = CONTROL_VALUE_DWORD,
												   .dwordValue = &GetState()->options.anisotropy,
											   },
											   1.0f,
											   1.0f,
											   SliderLabelAnisotropy,
											   "Improves the appearance of textures viewed at sharp angles. Requires "
											   "mipmaps to be enabled."));
		opY += opSpacing;

		UiStackPush(videoOptionsStack, CreateHeaderFooterControl(100, true, "Video Options"));
		UiStackPush(videoOptionsStack, CreateHeaderFooterControl(100, false, NULL));
		UiStackPush(videoOptionsStack,
					CreateButtonControl(v2(0, -40), v2(480, 40), "Back", BtnVideoOptionsBack, BOTTOM_CENTER, NULL));
	}
	UiStackResetFocus(videoOptionsStack);
	hasChangedVideoOptions = false;
}

static void VideoOptionsStateDestroy()
{
	if (videoOptionsStack != NULL)
	{
		FreeScrollView(videoOptionsScrollView);
		DestroyUiStack(videoOptionsStack);
		videoOptionsStack = NULL;
	}
}

const GameState VideoOptionsState = {
	.UpdateGame = VideoOptionsStateUpdate,
	.RenderGame = VideoOptionsStateRender,
	.FixedUpdateGame = OptionsStateFixedUpdate,
	.Destroy = VideoOptionsStateDestroy,
	.Set = VideoOptionsStateSet,
	.enableRelativeMouseMode = false,
};
