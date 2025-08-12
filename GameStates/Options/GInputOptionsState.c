//
// Created by droc101 on 11/23/2024.
//

#include "GInputOptionsState.h"
#include "../../Helpers/CommonAssets.h"
#include "../../Helpers/Core/Input.h"
#include "../../Helpers/Graphics/Drawing.h"
#include "../../Helpers/Graphics/Font.h"
#include "../../Helpers/Graphics/RenderingHelpers.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/UI/Controls/Button.h"
#include "../../Structs/UI/Controls/CheckBox.h"
#include "../../Structs/UI/Controls/Slider.h"
#include "../../Structs/UI/UiStack.h"
#include "../../Structs/Vector2.h"
#include "../GOptionsState.h"

UiStack *inputOptionsStack = NULL;

void BtnInputOptionsBack()
{
	GOptionsStateSet(optionsStateInGame);
}

void GInputOptionsStateUpdate(GlobalState * /*State*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(CONTROLLER_CANCEL))
	{
		BtnInputOptionsBack();
	}
}

void SldOptionsMouseSensitivity(const double value)
{
	GetState()->options.cameraSpeed = value;
}

void SldOptionsRumbleStrength(const double value)
{
	GetState()->options.rumbleStrength = (float)value;
	Rumble(1.0f, 200);
}

void CbOptionsControllerMode(const bool value)
{
	GetState()->options.controllerMode = value;
}

void CbOptionsInvertCameraH(const bool value)
{
	GetState()->options.invertHorizontalCamera = value;
}

void CbOptionsInvertCameraV(const bool value)
{
	GetState()->options.invertVerticalCamera = value;
}

void CbOptionsSwapOkCancel(const bool value)
{
	GetState()->options.controllerSwapOkCancel = value;
}

void GInputOptionsStateRender(GlobalState *)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground();
	}

	DrawTextAligned("Input Options",
					32,
					COLOR_WHITE,
					v2s(0),
					v2(WindowWidthFloat(), 100),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					largeFont);

	ProcessUiStack(inputOptionsStack);
	DrawUiStack(inputOptionsStack);

	DrawTextAligned("Controller Options",
					16,
					COLOR_WHITE,
					v2(0, 260),
					v2(WindowWidthFloat(), 40),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					smallFont);

	if (GetState()->options.controllerMode)
	{
		DrawTextAligned("Controller Name:",
						12,
						COLOR_WHITE,
						v2(0, 460),
						v2(WindowWidthFloat(), 40),
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
						v2(WindowWidthFloat(), 40),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						smallFont);
	}
}

void GInputOptionsStateSet()
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
					CreateCheckboxControl(v2(0, opY),
										  v2(480, 40),
										  "Controller Mode",
										  CbOptionsControllerMode,
										  TOP_CENTER,
										  GetState()->options.controllerMode));
		opY += opSpacing;
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

	SetStateCallbacks(GInputOptionsStateUpdate,
					  NULL,
					  INPUT_OPTIONS_STATE,
					  GInputOptionsStateRender); // Fixed update is not needed for this state
}

void GInputOptionsStateDestroy()
{
	if (inputOptionsStack != NULL)
	{
		DestroyUiStack(inputOptionsStack);
		inputOptionsStack = NULL;
	}
}
