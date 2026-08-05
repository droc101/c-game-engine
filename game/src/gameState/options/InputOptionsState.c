//
// Created by droc101 on 11/23/2024.
//

#include "gameState/options/InputOptionsState.h"
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
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/HeaderFooterControl.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/controls/OptionsButton.h>
#include <engine/uiStack/controls/Slider.h>
#include <engine/uiStack/ScrollView.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "gameState/options/ControlsOptionsState.h"
#include "gameState/OptionsState.h"

static UiStack *inputOptionsStack = NULL;
static ScrollView *inputOptionsScrollView = NULL;

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

	inputOptionsScrollView->size.y = ScaledWindowHeightFloat() - 200;
	ProcessScrollView(inputOptionsScrollView);
	ProcessUiStack(inputOptionsStack);
	DrawUiStack(inputOptionsStack);

	const char *controllerName = GetControllerName();
	if (!controllerName)
	{
		controllerName = "No Controller Connected";
	}
	snprintf(controllerNameBuffer, 128, "Controller: %s", controllerName);
}

static void InputOptionsStateSet()
{
	if (inputOptionsStack == NULL)
	{
		inputOptionsStack = CreateUiStack();
		inputOptionsScrollView = CreateScrollView(inputOptionsStack, TOP_CENTER, v2(0, 100), v2(750, 200));
		float opY = 0;
		const float opSpacing = 45;
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateButtonControl(v2(0, opY),
											   v2(750, 40),
											   "Edit Controls",
											   BtnControlsOptions,
											   TOP_CENTER,
											   NULL));
		opY += opSpacing * 1.5f;
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateLabelControl("Camera Options",
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
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
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
		opY += opSpacing;
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateOptionsButtonControl(v2(-190, opY),
													  v2(370, 40),
													  "Horizontal Camera: %s",
													  NULL,
													  TOP_CENTER,
													  invertCameraButtonValues,
													  2,
													  NULL,
													  (ControlValue){
														  .type = CONTROL_VALUE_BOOL,
														  .boolValue = &GetState()->options.invertHorizontalCamera,
													  },
													  NULL));
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateOptionsButtonControl(v2(190, opY),
													  v2(370, 40),
													  "Vertical Camera: %s",
													  NULL,
													  TOP_CENTER,
													  invertCameraButtonValues,
													  2,
													  NULL,
													  (ControlValue){
														  .type = CONTROL_VALUE_BOOL,
														  .boolValue = &GetState()->options.invertVerticalCamera,
													  },
													  NULL));
		opY += opSpacing * 1.5f;
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateLabelControl("Controller Options",
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
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateSliderControl(v2(-190, opY),
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
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateSliderControl(v2(190, opY),
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
		opY += opSpacing;
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateOptionsButtonControl(v2(0, opY),
													  v2(750, 40),
													  "Swap controller OK/Cancel buttons: %s",
													  NULL,
													  TOP_CENTER,
													  yesNoButtonValues,
													  2,
													  NULL,
													  (ControlValue){
														  .type = CONTROL_VALUE_BOOL,
														  .boolValue = &GetState()->options.controllerSwapOkCancel,
													  },
													  NULL));
		opY += opSpacing;
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateLabelControl(controllerNameBuffer,
											  16,
											  COLOR_WHITE,
											  v2(0, opY),
											  v2(750, 40),
											  TOP_CENTER,
											  FONT_HALIGN_CENTER,
											  FONT_VALIGN_MIDDLE,
											  FONT("small_font"),
											  true));
		opY += opSpacing;

		UiStackPush(inputOptionsStack, CreateHeaderFooterControl(100, true, "Input Options"));
		UiStackPush(inputOptionsStack, CreateHeaderFooterControl(100, false, NULL));
		UiStackPush(inputOptionsStack,
					CreateButtonControl(v2(0, -40), v2(480, 40), "Back", BtnInputOptionsBack, BOTTOM_CENTER, NULL));
	}
	UiStackResetFocus(inputOptionsStack);
}

static void InputOptionsStateDestroy()
{
	if (inputOptionsStack != NULL)
	{
		FreeScrollView(inputOptionsScrollView);
		DestroyUiStack(inputOptionsStack);
		inputOptionsStack = NULL;
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
