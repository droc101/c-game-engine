//
// Created by NBT22 on 9/1/26.
//

#include "gameState/RenderingTogglesState.h"
#include <engine/graphics/Drawing.h>
#include <engine/graphics/vulkan/VulkanDebug.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/helpers/Macros.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Options.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/OptionsButton.h>
#include <engine/uiStack/controls/Slider.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "gameState/OptionsState.h"
#include "helpers/OptionsMenu.h"

static OptionsMenu *renderingTogglesMenu;
static bool enableBakedLighting = true;
static DebugRendering debugRendering = DEBUG_RENDERING_DISABLED;

static void ToggleBakedLightingCallback(const OptionsButtonValue * /*value*/, void * /*extraData*/)
{
	VK_ToggleBakedLighting();
}

static void SetDebugRenderingCallback(const ControlValue * /*value*/)
{
	VK_SetDebugRendering(debugRendering);
}

static char *SliderLabelDebugRenderingType(const Control *slider)
{
	const char *types[] = {
		"Off",
		"Disable lighting",
		"Untextured",
		"Normals",
		"UVs",
		"Clusters",
		"Cluster light count",
		"Only lighting",
		"No light falloff",
		"Depth",
		"Primitives",
		"Depth Pre-Pass Disabled",
		"Culling spheres",
	};
	const SliderData *data = (SliderData *)slider->controlData;
	char *buffer = malloc(64);
	CheckAlloc(buffer);
	snprintf(buffer,
			 64,
			 "%s: %s",
			 data->label,
			 types[min((uint32_t)GetSliderValueAsFloat(data), ArrayLength(types) - 1)]);
	buffer[63] = '\0';
	return buffer;
}

static char *SliderLabelCullingSpheresFilterType(const Control *slider)
{
	const char *types[] = {"None", "Shaded", "Unshaded", "Both"};
	const SliderData *data = (SliderData *)slider->controlData;
	char *buffer = malloc(64);
	CheckAlloc(buffer);
	snprintf(buffer,
			 64,
			 "%s: %s",
			 data->label,
			 types[min((uint32_t)GetSliderValueAsFloat(data), ArrayLength(types) - 1)]);
	buffer[63] = '\0';
	return buffer;
}

static Control *CreateCullingSpheresFilterSlider(char *label, uint32_t *filter)
{
	return CreateSliderControl(v2s(0),
							   v2s(0),
							   label,
							   NULL,
							   TOP_CENTER,
							   CULLING_SPHERES_FILTER_NONE,
							   CULLING_SPHERES_FILTER_BOTH,
							   (ControlValue){
								   .type = CONTROL_VALUE_DWORD,
								   .dwordValue = filter,
							   },
							   1.0f,
							   1.0f,
							   SliderLabelCullingSpheresFilterType,
							   NULL);
}

static void BtnRenderingTogglesBack(Control * /*control*/, void * /*extraData*/)
{
	SaveOptions(&GetState()->options);
	SetGameState(&OptionsState);
}

static void RenderingTogglesStateUpdate(GlobalState *state, const double delta)
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL))
	{
		BtnRenderingTogglesBack(NULL, NULL);
	}
	if (!optionsStateInGame)
	{
		UpdateMenuBackground(state, delta);
	}
}

static void RenderingTogglesStateRender(GlobalState *state, const double /*delta*/)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground(state, true);
	}

	ProcessOptionsMenu(renderingTogglesMenu);
}

static void RenderingTogglesStateSet(void)
{
	if (renderingTogglesMenu == NULL)
	{
		renderingTogglesMenu = CreateOptionsMenu();

		OptionsMenuAddLargeControl(renderingTogglesMenu,
								   CreateOptionsButtonControl(v2s(0),
															  v2s(0),
															  "Baked lighting: %s",
															  ToggleBakedLightingCallback,
															  TOP_CENTER,
															  &onOffButtonOptions,
															  NULL,
															  (ControlValue){
																  .type = CONTROL_VALUE_BOOL,
																  .boolValue = &enableBakedLighting,
															  },
															  NULL));

		OptionsMenuAddLargeControl(renderingTogglesMenu,
								   CreateSliderControl(v2s(0),
													   v2s(0),
													   "Debug rendering option",
													   SetDebugRenderingCallback,
													   TOP_CENTER,
													   DEBUG_RENDERING_DISABLED,
													   DEBUG_RENDERING_COUNT - 1,
													   (ControlValue){
														   .type = CONTROL_VALUE_DWORD,
														   .dwordValue = (uint32_t *)&debugRendering,
													   },
													   1.0f,
													   1.0f,
													   SliderLabelDebugRenderingType,
													   NULL));

		OptionsMenuAddSection(renderingTogglesMenu, "Culling Spheres");
		OptionsMenuAddLargeControl(renderingTogglesMenu,
								   CreateCullingSpheresFilterSlider("Opaque map geometry",
																	&cullingSpheresFilters.opaqueMapModels));
		OptionsMenuAddLargeControl(renderingTogglesMenu,
								   CreateCullingSpheresFilterSlider("Translucent map geometry",
																	&cullingSpheresFilters.translucentMapModels));
		OptionsMenuAddLargeControl(renderingTogglesMenu,
								   CreateCullingSpheresFilterSlider("Actor models",
																	&cullingSpheresFilters.actorModels));
		OptionsMenuAddLargeControl(renderingTogglesMenu,
								   CreateCullingSpheresFilterSlider("Actor walls", &cullingSpheresFilters.actorWalls));

		OptionsMenuAddSimpleHeaderFooter(renderingTogglesMenu, "Rendering Toggles", BtnRenderingTogglesBack);
	}
	UiStackResetFocus(renderingTogglesMenu->stack);
}

static void RenderingTogglesStateDestroy(void)
{
	if (renderingTogglesMenu != NULL)
	{
		DestroyOptionsMenu(renderingTogglesMenu);
		renderingTogglesMenu = NULL;
	}
}

const GameState RenderingTogglesState = {
	.UpdateGame = RenderingTogglesStateUpdate,
	.RenderGame = RenderingTogglesStateRender,
	.FixedUpdateGame = OptionsStateFixedUpdate,
	.Destroy = RenderingTogglesStateDestroy,
	.Set = RenderingTogglesStateSet,
	.enableRelativeMouseMode = false,
};
