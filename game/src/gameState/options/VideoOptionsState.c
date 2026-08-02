//
// Created by droc101 on 11/23/2024.
//

#include "gameState/options/VideoOptionsState.h"
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Options.h>
#include <engine/structs/Vector2.h>
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "gameState/options/RestartPromptState.h"
#include "gameState/OptionsState.h"

static UiStack *videoOptionsStack = NULL;
static ScrollView *videoOptionsScrollView = NULL;
static bool hasChangedVideoOptions = false;

static OptionsButtonValue gpuTypeButtonValues[3] = {
	{
		.text = "Dedicated",
		.tooltip = "Prefer using a dedicated GPU, which usually provides the best performance.",
		.value = GPU_TYPE_DEDICATED,
	},
	{
		.text = "Integrated",
		.tooltip = "Prefer using an integrated GPU, which usually provides less performance than a dedicated GPU.",
		.value = GPU_TYPE_INTEGRATED,
	},
	{
		.text = "Software",
		.tooltip = "Prefer using a software emulated GPU, which usually provides the worst performance.",
		.value = GPU_TYPE_SOFTWARE,
	},
};

static OptionsButtonValue preferWaylandButtonValues[2] = {
	{
		.text = "Prefer X11",
		.tooltip = NULL,
		.value = false,
	},
	{
		.text = "Prefer Wayland",
		.tooltip = NULL,
		.value = true,
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
	char *labels[] = {"Disabled", "128px", "256px", "512px", "1024px", "2048px", "4096px"};
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %s", data->label, labels[(int)data->value]);
	return buf;
}

static char *SliderLabelMSAA(const Control *slider)
{
	char *labels[] = {"Off", "2x MSAA", "4x MSAA", "8x MSAA"};
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %s", data->label, labels[(int)data->value]);
	return buf;
}

static char *SliderLabelAnisotropy(const Control *slider)
{
	char *labels[] = {"Off", "2x", "4x", "8x", "16x"};
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %s", data->label, labels[(int)data->value]);
	return buf;
}

static char *SliderLabelLod(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	sprintf(buf, "%s: %.1fx", data->label, data->value);
	return buf;
}

static char *SliderLabelMaxFps(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->controlData;
	char *buf = malloc(64);
	CheckAlloc(buf);
	if (data->value == 0)
	{
		sprintf(buf, "%s: Unlimited", data->label);
	} else
	{
		sprintf(buf, "%s: %.0f", data->label, data->value);
	}
	return buf;
}

static void OptBtnFullscreenChanged(size_t value, void *)
{
	GetState()->options.fullscreen = onOffButtonValues[value].value;
	SDL_SetWindowFullscreen(GetGameWindow(), onOffButtonValues[value].value);
}

static void OptBtnVsyncChanged(size_t value, void *)
{
	GetState()->options.vsync = onOffButtonValues[value].value;
	hasChangedVideoOptions = true; // Until Luna can do this
	rendererQueuedActions |= QUEUED_ACTION_TOGGLE_VSYNC;
}

static void OptBtnLimitFpsWhenUnfocusedChanged(size_t value, void *)
{
	GetState()->options.limitFpsWhenUnfocused = yesNoButtonValues[value].value;
}

static void OptButtonMipmapsChanged(const size_t value, void * /*extraData*/)
{
	GetState()->options.mipmaps = onOffButtonValues[value].value;
	rendererQueuedActions |= QUEUED_ACTION_CLEAR_ALL_TEXTURES;
}

static void SldOptionsShadowMapQuality(const float value)
{
	GetState()->options.shadowMapQuality = value;
	rendererQueuedActions |= QUEUED_ACTION_UPDATE_SHADOW_MAP_RESOLUTION;
}

static void CbOptionsPreferWayland(size_t value, void *)
{
	GetState()->options.preferWayland = preferWaylandButtonValues[value].value;
	hasChangedVideoOptions = true;
	// Change will happen next restart
}

static void SldOptionsMsaa(const float value)
{
	GetState()->options.msaa = value;
	hasChangedVideoOptions = true; // Until Luna can do this
	rendererQueuedActions |= QUEUED_ACTION_UPDATE_MSAA;
}

static void SldOptionsAnisotropy(const float value)
{
	GetState()->options.anisotropy = value;
	rendererQueuedActions |= QUEUED_ACTION_CLEAR_ALL_TEXTURES;
}

static void SldOptionsLod(const float value)
{
	GetState()->options.lodMultiplier = value;
}

static void SldOptionsMaxFps(const float value)
{
	GetState()->options.maxFps = (uint16_t)value;
}

static void SldOptionsFov(const float value)
{
	GetState()->options.fov = value;
	if (GetState()->map)
	{
		GetState()->map->player.playerCamera.fov = GetState()->options.fov;
	}
}

static void OptBtnPreferredGpuTypeChanged(const size_t value, void * /*extraData*/)
{
	GetState()->options.preferredGpuType = gpuTypeButtonValues[value].value;
	hasChangedVideoOptions = true;
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
											   SldOptionsFov,
											   TOP_CENTER,
											   30.0,
											   120.0,
											   GetState()->options.fov,
											   1,
											   1,
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
											  smallFont,
											  true));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
											   v2(750, 40),
											   "Maximum FPS",
											   SldOptionsMaxFps,
											   TOP_CENTER,
											   0,
											   500,
											   GetState()->options.maxFps,
											   10,
											   10,
											   SliderLabelMaxFps,
											   NULL));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateOptionsButtonControl(v2(-190, opY),
													  v2(370, 40),
													  "VSync: %s",
													  OptBtnVsyncChanged,
													  TOP_CENTER,
													  onOffButtonValues,
													  2,
													  NULL,
													  GetState()->options.vsync,
													  "Limits the framerate to your monitor to reduce screen "
													  "tearing"));
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateOptionsButtonControl(v2(190, opY),
													  v2(370, 40),
													  "Limit Background FPS: %s",
													  OptBtnLimitFpsWhenUnfocusedChanged,
													  TOP_CENTER,
													  yesNoButtonValues,
													  2,
													  NULL,
													  GetState()->options.limitFpsWhenUnfocused,
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
													  GetState()->options.fullscreen,
													  NULL));
#ifdef SDL_PLATFORM_LINUX
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateOptionsButtonControl(v2(190, opY),
													  v2(370, 40),
													  "Video Platform: %s",
													  CbOptionsPreferWayland,
													  TOP_CENTER,
													  preferWaylandButtonValues,
													  2,
													  NULL,
													  GetState()->options.preferWayland,
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
											  smallFont,
											  true));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateOptionsButtonControl(v2(0, opY),
													  v2(750, 40),
													  "Preferred GPU Type: %s",
													  OptBtnPreferredGpuTypeChanged,
													  TOP_CENTER,
													  gpuTypeButtonValues,
													  3,
													  NULL,
													  GetState()->options.preferredGpuType,
													  NULL));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
											   v2(750, 40),
											   "Shadow Resolution",
											   SldOptionsShadowMapQuality,
											   TOP_CENTER,
											   0.0,
											   6.0,
											   GetState()->options.shadowMapQuality,
											   1,
											   1,
											   SliderLabelShadowMapQuality,
											   "The resolution to use for shadow maps"));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
											   v2(750, 40),
											   "Anti-Aliasing",
											   SldOptionsMsaa,
											   TOP_CENTER,
											   0.0,
											   3.0,
											   GetState()->options.msaa,
											   1,
											   1,
											   SliderLabelMSAA,
											   "Smooths the edges of objects"));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
											   v2(750, 40),
											   "LOD Distance",
											   SldOptionsLod,
											   TOP_CENTER,
											   0.5,
											   2.0,
											   GetState()->options.lodMultiplier,
											   0.5,
											   1,
											   SliderLabelLod,
											   "Changes the level of detail on far away objects"));
		opY += opSpacing;
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateOptionsButtonControl(v2(-190, opY),
													  v2(370, 40),
													  "Mipmaps: %s",
													  OptButtonMipmapsChanged,
													  TOP_CENTER,
													  onOffButtonValues,
													  2,
													  NULL,
													  GetState()->options.mipmaps,
													  NULL));
		ScrollViewAddChild(videoOptionsScrollView,
						   CreateSliderControl(v2(190, opY),
											   v2(370, 40),
											   "Anisotropic Filtering",
											   SldOptionsAnisotropy,
											   TOP_CENTER,
											   0.0,
											   4.0,
											   GetState()->options.anisotropy,
											   1,
											   1,
											   SliderLabelAnisotropy,
											   "Improves the appearance of textures viewed at sharp angles"));
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
