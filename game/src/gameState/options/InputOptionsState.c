//
// Created by droc101 on 11/23/2024.
//

#include "gameState/options/InputOptionsState.h"
#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Options.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/controls/OptionsButton.h>
#include <engine/uiStack/controls/Slider.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "gameState/options/ControlsOptionsState.h"
#include "gameState/OptionsState.h"
#include "helpers/OptionsMenu.h"

static OptionsMenu *inputOptionsMenu;

static char controllerNameBuffer[256];

static OptionsButtonValue invertCameraButtonValues[2] = {
	{
		.text = "Normal",
		.tooltip = NULL,
		.value =
				{
					.type = CONTROL_VALUE_BOOL,
					.boolValue = false,
				},
	},
	{
		.text = "Inverted",
		.tooltip = NULL,
		.value =
				{
					.type = CONTROL_VALUE_BOOL,
					.boolValue = true,
				},
	},
};

static void BtnInputOptionsBack(Control *, void *)
{
	SaveOptions(&GetState()->options);
	SetGameState(&OptionsState);
}

static void InputOptionsStateUpdate(GlobalState *state, const double delta)
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL))
	{
		BtnInputOptionsBack(NULL, NULL);
	}
	if (!optionsStateInGame)
	{
		UpdateMenuBackground(state, delta);
	}
}

static void BtnControlsOptions(Control *, void *)
{
	SetGameState(&ControlsOptionsState);
}

static void InputOptionsStateRender(GlobalState *state, const double /*delta*/)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground(state);
	}

	ProcessOptionsMenu(inputOptionsMenu);

	const char *controllerName = GetControllerName();
	if (!controllerName)
	{
		controllerName = "No Controller Connected";
	}
	snprintf(controllerNameBuffer, 128, "Controller: %s", controllerName);
}

static void InputOptionsStateSet()
{
	if (inputOptionsMenu == NULL)
	{
		inputOptionsMenu = CreateOptionsMenu();
		OptionsMenuAddLargeControl(inputOptionsMenu,
								   CreateButtonControl(v2(0, 0),
													   v2(750, 40),
													   "Edit Controls",
													   BtnControlsOptions,
													   TOP_CENTER,
													   NULL));
		OptionsMenuAddSection(inputOptionsMenu, "Camera Options");
		OptionsMenuAddLargeControl(inputOptionsMenu,
								   CreateSliderControl(v2(0, 0),
													   v2(750, 40),
													   "Camera Sensitivity",
													   NULL,
													   TOP_CENTER,
													   0.01f,
													   2.00f,
													   (ControlValue){
														   .type = CONTROL_VALUE_FLOAT,
														   .floatValue = &GetState()->options.cameraSpeed,
													   },
													   0.01f,
													   0.1f,
													   SliderLabelPercent,
													   NULL));
		OptionsMenuAddSmallControl(inputOptionsMenu,
								   CreateOptionsButtonControl(v2(-190, 0),
															  v2(370, 40),
															  "Horizontal Camera: %s",
															  NULL,
															  TOP_CENTER,
															  invertCameraButtonValues,
															  2,
															  NULL,
															  (ControlValue){
																  .type = CONTROL_VALUE_BOOL,
																  .boolValue = &GetState()
																						->options
																						.invertHorizontalCamera,
															  },
															  NULL));
		OptionsMenuAddSmallControl(inputOptionsMenu,
								   CreateOptionsButtonControl(v2(190, 0),
															  v2(370, 40),
															  "Vertical Camera: %s",
															  NULL,
															  TOP_CENTER,
															  invertCameraButtonValues,
															  2,
															  NULL,
															  (ControlValue){
																  .type = CONTROL_VALUE_BOOL,
																  .boolValue = &GetState()
																						->options.invertVerticalCamera,
															  },
															  NULL));
		OptionsMenuAddSection(inputOptionsMenu, "Controller Options");
		OptionsMenuAddSmallControl(inputOptionsMenu,
								   CreateSliderControl(v2(-190, 0),
													   v2(370, 40),
													   "Rumble Strength",
													   NULL,
													   TOP_CENTER,
													   0.0f,
													   1.0f,
													   (ControlValue){
														   .type = CONTROL_VALUE_FLOAT,
														   .floatValue = &GetState()->options.rumbleStrength,
													   },
													   0.25f,
													   0.25f,
													   SliderLabelPercent,
													   NULL));
		OptionsMenuAddSmallControl(inputOptionsMenu,
								   CreateSliderControl(v2(190, 0),
													   v2(370, 40),
													   "Stick Deadzone",
													   NULL,
													   TOP_CENTER,
													   0.0f,
													   0.5f,
													   (ControlValue){
														   .type = CONTROL_VALUE_FLOAT,
														   .floatValue = &GetState()->options.controllerDeadzone,
													   },
													   0.01f,
													   0.01f,
													   NULL,
													   NULL));
		OptionsMenuAddLargeControl(inputOptionsMenu,
								   CreateOptionsButtonControl(v2(0, 0),
															  v2(750, 40),
															  "Swap controller OK/Cancel buttons: %s",
															  NULL,
															  TOP_CENTER,
															  yesNoButtonValues,
															  2,
															  NULL,
															  (ControlValue){
																  .type = CONTROL_VALUE_BOOL,
																  .boolValue = &GetState()
																						->options
																						.controllerSwapOkCancel,
															  },
															  NULL));
		OptionsMenuAddLargeControl(inputOptionsMenu,
								   CreateLabelControl(controllerNameBuffer,
													  16,
													  COLOR_WHITE,
													  v2(0, 0),
													  v2(750, 40),
													  TOP_CENTER,
													  FONT_HALIGN_CENTER,
													  FONT_VALIGN_MIDDLE,
													  FONT("small_font"),
													  true));

		OptionsMenuAddHeaderFooter(inputOptionsMenu, "Input Options", BtnInputOptionsBack);
	}
	UiStackResetFocus(inputOptionsMenu->stack);
}

static void InputOptionsStateDestroy()
{
	if (inputOptionsMenu != NULL)
	{
		DestroyOptionsMenu(inputOptionsMenu);
		inputOptionsMenu = NULL;
	}
}

const GameState InputOptionsState = {
	.UpdateGame = InputOptionsStateUpdate,
	.RenderGame = InputOptionsStateRender,
	.FixedUpdateGame = OptionsStateFixedUpdate,
	.Destroy = InputOptionsStateDestroy,
	.Set = InputOptionsStateSet,
	.enableRelativeMouseMode = false,
};
