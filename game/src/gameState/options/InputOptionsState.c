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
#include <engine/uiStack/controls/Slider.h>
#include <engine/uiStack/UiStack.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include "gameState/OptionsState.h"

static UiStack *inputOptionsStack = NULL;

static void BtnInputOptionsBack()
{
	SaveOptions(&GetState()->options);
	SetGameState(&OptionsState);
}

static void InputOptionsStateUpdate(GlobalState *state, const double delta)
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL))
	{
		BtnInputOptionsBack();
	}
	if (!optionsStateInGame)
	{
		UpdateMenuBackground(state, delta);
	}
}

static void SldOptionsMouseSensitivity(const float value)
{
	GetState()->options.cameraSpeed = value;
}

static void SldOptionsRumbleStrength(const float value)
{
	GetState()->options.rumbleStrength = value;
	Rumble(1.0f, 200, mainThreadInput);
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

	DrawTextAligned("Input Options",
					32,
					COLOR_WHITE,
					v2s(0),
					v2(ScaledWindowWidthFloat(), 100),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					largeFont);

	ProcessUiStack(inputOptionsStack);
	DrawUiStack(inputOptionsStack);

	DrawTextAligned("Controller Options",
					16,
					COLOR_WHITE,
					v2(0, 260),
					v2(ScaledWindowWidthFloat(), 40),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					smallFont);

	{
		DrawTextAligned("Controller Name:",
						12,
						COLOR_WHITE,
						v2(0, 460),
						v2(ScaledWindowWidthFloat(), 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						smallFont);
		const char *controllerName = GetControllerName();
		if (!controllerName)
		{
			controllerName = "No Controller Connected";
		}
		DrawTextAligned(controllerName,
						12,
						COLOR_WHITE,
						v2(0, 480),
						v2(ScaledWindowWidthFloat(), 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						smallFont);
	}
}

static void InputOptionsStateSet()
{
	if (inputOptionsStack == NULL)
	{
		inputOptionsStack = CreateUiStack();
		float opY = 80;
		const float opSpacing = 45;

		UiStackPush(inputOptionsStack,
					CreateSliderControl(v2(0, opY),
										v2(480, 40),
										"Camera Sensitivity",
										SldOptionsMouseSensitivity,
										TOP_CENTER,
										0.01,
										2.00,
										GetState()->options.cameraSpeed,
										0.01,
										0.1,
										SliderLabelPercent));
		opY += opSpacing;
		UiStackPush(inputOptionsStack,
					CreateCheckboxControl(v2(0, opY),
										  v2(480, 40),
										  "Invert Horizontal Camera",
										  CbOptionsInvertCameraH,
										  TOP_CENTER,
										  GetState()->options.invertHorizontalCamera));
		opY += opSpacing;
		UiStackPush(inputOptionsStack,
					CreateCheckboxControl(v2(0, opY),
										  v2(480, 40),
										  "Invert Vertical Camera",
										  CbOptionsInvertCameraV,
										  TOP_CENTER,
										  GetState()->options.invertVerticalCamera));
		opY += opSpacing * 3;
		UiStackPush(inputOptionsStack,
					CreateSliderControl(v2(0, opY),
										v2(480, 40),
										"Rumble Strength",
										SldOptionsRumbleStrength,
										TOP_CENTER,
										0.0,
										1.0,
										GetState()->options.rumbleStrength,
										0.25,
										0.25,
										SliderLabelPercent));
		opY += opSpacing;
		UiStackPush(inputOptionsStack,
					CreateCheckboxControl(v2(0, opY),
										  v2(480, 40),
										  "Swap OK/Cancel buttons",
										  CbOptionsSwapOkCancel,
										  TOP_CENTER,
										  GetState()->options.controllerSwapOkCancel));
		opY += opSpacing;


		UiStackPush(inputOptionsStack,
					CreateButtonControl(v2(0, -40), v2(480, 40), "Back", BtnInputOptionsBack, BOTTOM_CENTER));
	}
	UiStackResetFocus(inputOptionsStack);
}

static void InputOptionsStateDestroy()
{
	if (inputOptionsStack != NULL)
	{
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
