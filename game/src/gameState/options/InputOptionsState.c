//
// Created by droc101 on 11/23/2024.
//

#include "gameState/options/InputOptionsState.h"
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
#include <engine/uiStack/controls/CheckBox.h>
#include <engine/uiStack/controls/HeaderFooterControl.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/controls/Slider.h>
#include <engine/uiStack/ScrollView.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include "gameState/options/ControlsOptionsState.h"
#include "gameState/OptionsState.h"

static UiStack *inputOptionsStack = NULL;
static ScrollView *inputOptionsScrollView = NULL;

static char controllerNameBuffer[256];

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

static void SldOptionsRumbleStrength(const float value)
{
	GetState()->options.rumbleStrength = value;
	Rumble(1.0f, 200, mainThreadInput);
}

static void SldOptionsStickDeadzone(const float value)
{
	GetState()->options.controllerDeadzone = value;
}

static void SldOptionsMouseSensitivity(const float value)
{
	GetState()->options.cameraSpeed = value;
}

static void CbOptionsInvertCameraH(const bool value)
{
	GetState()->options.invertHorizontalCamera = value;
}

static void CbOptionsInvertCameraV(const bool value)
{
	GetState()->options.invertVerticalCamera = value;
}

static void CbOptionsSwapOkCancel(const bool value)
{
	GetState()->options.controllerSwapOkCancel = value;
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
											  FONT_HALIGN_CENTER,
											  FONT_VALIGN_MIDDLE,
											  smallFont,
											  true));
		opY += opSpacing;
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateSliderControl(v2(0, opY),
											   v2(750, 40),
											   "Camera Sensitivity",
											   SldOptionsMouseSensitivity,
											   TOP_CENTER,
											   0.01,
											   2.00,
											   GetState()->options.cameraSpeed,
											   0.01,
											   0.1,
											   SliderLabelPercent,
											   NULL));
		opY += opSpacing;
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateCheckboxControl(v2(-185, opY),
												 v2(370, 40),
												 "Invert Horizontal Camera",
												 CbOptionsInvertCameraH,
												 TOP_CENTER,
												 GetState()->options.invertHorizontalCamera,
												 NULL));
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateCheckboxControl(v2(190, opY),
												 v2(370, 40),
												 "Invert Vertical Camera",
												 CbOptionsInvertCameraV,
												 TOP_CENTER,
												 GetState()->options.invertVerticalCamera,
												 NULL));
		opY += opSpacing * 1.5f;
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateLabelControl("Controller Options",
											  16,
											  COLOR_WHITE,
											  v2(0, opY),
											  v2(750, 40),
											  TOP_CENTER,
											  FONT_HALIGN_CENTER,
											  FONT_VALIGN_MIDDLE,
											  smallFont,
											  true));
		opY += opSpacing;
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateSliderControl(v2(-185, opY),
											   v2(370, 40),
											   "Rumble Strength",
											   SldOptionsRumbleStrength,
											   TOP_CENTER,
											   0.0,
											   1.0,
											   GetState()->options.rumbleStrength,
											   0.25,
											   0.25,
											   SliderLabelPercent,
											   NULL));
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateSliderControl(v2(190, opY),
											   v2(370, 40),
											   "Stick Deadzone",
											   SldOptionsStickDeadzone,
											   TOP_CENTER,
											   0.0,
											   0.5,
											   GetState()->options.controllerDeadzone,
											   0.01,
											   0.01,
											   NULL,
											   NULL));
		opY += opSpacing;
		ScrollViewAddChild(inputOptionsScrollView,
						   CreateCheckboxControl(v2(0, opY),
												 v2(750, 40),
												 "Swap controller OK/Cancel buttons",
												 CbOptionsSwapOkCancel,
												 TOP_CENTER,
												 GetState()->options.controllerSwapOkCancel,
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
											  smallFont,
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
