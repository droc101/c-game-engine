//
// Created by NBT22 on 9/1/26.
//

#include "gameState/RenderingTogglesState.h"
#include <engine/graphics/Drawing.h>
#include <engine/graphics/vulkan/VulkanDebug.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Options.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/OptionsButton.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include "gameState/OptionsState.h"
#include "helpers/OptionsMenu.h"

static OptionsMenu *renderingTogglesMenu;
static bool enableBakedLighting = true;

static void ToggleBakedLightingCallback(const OptionsButtonValue * /*value*/, void * /*extraData*/)
{
	VK_ToggleBakedLighting();
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

static void RenderingTogglesStateSet()
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
															  onOffButtonValues,
															  2,
															  NULL,
															  (ControlValue){
																  .type = CONTROL_VALUE_BOOL,
																  .boolValue = &enableBakedLighting,
															  },
															  NULL));

		OptionsMenuAddSimpleHeaderFooter(renderingTogglesMenu, "Rendering Toggles", BtnRenderingTogglesBack);
	}
	UiStackResetFocus(renderingTogglesMenu->stack);
}

static void RenderingTogglesStateDestroy()
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
