//
// Created by droc101 on 7/22/26.
//

#include "gameState/options/ControlsOptionsState.h"
#include <engine/assets/AssetReader.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/BackgroundMapManager.h>
#include <engine/structs/Color.h>
#include <engine/structs/ControlOptions.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/InputAction.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Input.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/HeaderFooterControl.h>
#include <engine/uiStack/controls/IconButton.h>
#include <engine/uiStack/controls/LabelControl.h>
#include <engine/uiStack/ScrollView.h>
#include <engine/uiStack/UiStack.h>
#include <gameState/OptionsState.h>
#include <math.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "engine/subsystem/Error.h"
#include "gameState/options/InputOptionsState.h"

typedef enum ListenMode
{
	NOT_LISTNENING,
	KBM_LISTEN,
	CTLR_LISTEN,
} ListenMode;

typedef struct ControlRowData
{
	ControlOption *option;
	Control *kbmButton;
	Control *ctlrButton;
	Control *resetButton;
} ControlRowData;

static UiStack *controlsOptionsStack = NULL;
static ScrollView *controlsOptionsScrollView = NULL;
static char *filter = NULL;

static const int ENTRY_HEIGHT = 44;

static ListenMode listenMode;
static ControlRowData *listenRow;

static List controlRows;

static void BtnControlsOptionsBack(Control *, void *)
{
	SetGameState(&InputOptionsState);
}

static void UpdateControlRow(ControlRowData *row)
{
	ButtonData *kbmButtonData = row->kbmButton->controlData;
	ButtonData *ctlrButtonData = row->ctlrButton->controlData;
	IconButtonData *resetButtonData = row->resetButton->controlData;

	if (listenRow == row && listenMode == KBM_LISTEN)
	{
		kbmButtonData->text = "...";
	} else
	{
		kbmButtonData->text = InputActionGetKbmString(row->option->action);
	}

	if (listenRow == row && listenMode == CTLR_LISTEN)
	{
		ctlrButtonData->text = "...";
	} else
	{
		ctlrButtonData->text = InputActionGetControllerString(row->option->action);
	}

	kbmButtonData->extraData = row;
	ctlrButtonData->extraData = row;
	resetButtonData->extraData = row;
}

static void BtnResetBinding(Control *, void *extraData)
{
	ControlRowData *row = extraData;
	listenMode = NOT_LISTNENING;
	memcpy(row->option->action, row->option->defaultAction, sizeof(InputAction));
	UpdateControlRow(row);
}

static void BtnEditKbmBinding(Control *, void *extraData)
{
	if (listenMode != NOT_LISTNENING)
	{
		return;
	}
	ControlRowData *row = extraData;
	listenMode = KBM_LISTEN;
	listenRow = row;
	UpdateControlRow(row);
}

static void BtnEditCtlrBinding(Control *, void *extraData)
{
	if (listenMode != NOT_LISTNENING)
	{
		return;
	}
	ControlRowData *row = extraData;
	listenMode = CTLR_LISTEN;
	listenRow = row;
	UpdateControlRow(row);
}

static void FilterTextChanged(const char *newFilter)
{
	free(filter);
	filter = NULL;
	if (newFilter && strlen(newFilter) > 0)
	{
		filter = strdup(newFilter);
	}
}

static void ProcessListening()
{
	if (listenMode != NOT_LISTNENING)
	{
		InputAction *action = listenRow->option->action;
		if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
			IsButtonJustPressed(mainThreadInput, SDL_GAMEPAD_BUTTON_START))
		{
			if (listenMode == KBM_LISTEN)
			{
				action->keyboardMouseBindType = IA_UNBOUND;
			} else if (listenMode == CTLR_LISTEN)
			{
				action->controllerBindType = IA_UNBOUND;
			}
			listenMode = NOT_LISTNENING;
			ConsumeKey(mainThreadInput, SDL_SCANCODE_ESCAPE);
			ConsumeButton(mainThreadInput, SDL_GAMEPAD_BUTTON_START);
			UpdateControlRow(listenRow);
		}
		if (listenMode == KBM_LISTEN)
		{
			const SDL_Scancode pressedScancode = GetPressedScancode(mainThreadInput);
			if (pressedScancode != SDL_SCANCODE_UNKNOWN)
			{
				action->keyboardMouseBindType = IA_KEY;
				action->keyboardMouseBind.keyBind = pressedScancode;
				listenMode = NOT_LISTNENING;
				ConsumeKey(mainThreadInput, pressedScancode);
				UpdateControlRow(listenRow);
				return;
			}

			const uint8_t pressedMouseButton = GetPressedMouseButton(mainThreadInput);
			if (pressedMouseButton != 0xFF)
			{
				action->keyboardMouseBindType = IA_MOUSE_BUTTON;
				action->keyboardMouseBind.mouseButtonBind = pressedMouseButton;
				listenMode = NOT_LISTNENING;
				ConsumeMouseButton(mainThreadInput, pressedMouseButton);
				UpdateControlRow(listenRow);
				return;
			}

			if (listenRow->option->allowAxisBind)
			{
				const Vector2 mouseWheel = GetMouseWheelTicks(mainThreadInput);
				if (fabsf(mouseWheel.x) > 0 || fabsf(mouseWheel.y) > 0)
				{
					action->keyboardMouseBindType = IA_MOUSE_WHEEL;
					if (mouseWheel.y > 0)
					{
						action->keyboardMouseBind.mouseWheelBind = MOUSE_WHEEL_UP;
					} else if (mouseWheel.y < 0)
					{
						action->keyboardMouseBind.mouseWheelBind = MOUSE_WHEEL_DOWN;
					} else if (mouseWheel.x > 0)
					{
						action->keyboardMouseBind.mouseWheelBind = MOUSE_WHEEL_RIGHT;
					} else if (mouseWheel.x < 0)
					{
						action->keyboardMouseBind.mouseWheelBind = MOUSE_WHEEL_LEFT;
					}
					listenMode = NOT_LISTNENING;
					ConsumeMouseWheel(mainThreadInput);
					UpdateControlRow(listenRow);
					return;
				}
			}
		} else if (listenMode == CTLR_LISTEN)
		{
			const SDL_GamepadButton pressedButton = GetPressedButton(mainThreadInput);
			if (pressedButton != SDL_GAMEPAD_BUTTON_INVALID)
			{
				action->controllerBindType = IA_CONTROLLER_BUTTON;
				action->controllerBind.buttonBind = pressedButton;
				listenMode = NOT_LISTNENING;
				ConsumeButton(mainThreadInput, pressedButton);
				UpdateControlRow(listenRow);
				return;
			}

			if (listenRow->option->allowAxisBind)
			{
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > 0.0f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = LEFT_TRIGGER;
					listenMode = NOT_LISTNENING;
					UpdateControlRow(listenRow);
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.0f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = RIGHT_TRIGGER;
					listenMode = NOT_LISTNENING;
					UpdateControlRow(listenRow);
					return;
				}

				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_LEFTY) > 0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = LEFT_STICK_DOWN;
					listenMode = NOT_LISTNENING;
					UpdateControlRow(listenRow);
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_LEFTY) < -0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = LEFT_STICK_UP;
					listenMode = NOT_LISTNENING;
					UpdateControlRow(listenRow);
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_LEFTX) > 0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = LEFT_STICK_RIGHT;
					listenMode = NOT_LISTNENING;
					UpdateControlRow(listenRow);
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_LEFTX) < -0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = LEFT_STICK_LEFT;
					listenMode = NOT_LISTNENING;
					UpdateControlRow(listenRow);
					return;
				}

				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_RIGHTY) > 0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = RIGHT_STICK_DOWN;
					listenMode = NOT_LISTNENING;
					UpdateControlRow(listenRow);
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_RIGHTY) < -0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = RIGHT_STICK_UP;
					listenMode = NOT_LISTNENING;
					UpdateControlRow(listenRow);
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_RIGHTX) > 0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = RIGHT_STICK_RIGHT;
					listenMode = NOT_LISTNENING;
					UpdateControlRow(listenRow);
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_RIGHTX) < -0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = RIGHT_STICK_LEFT;
					listenMode = NOT_LISTNENING;
					UpdateControlRow(listenRow);
					return;
				}
			}
		}
	}
}

static void ControlsOptionsStateUpdate(GlobalState *state, const double delta)
{
	ProcessListening();

	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, CONTROLLER_CANCEL))
	{
		BtnControlsOptionsBack(NULL, NULL);
	}
	if (!optionsStateInGame)
	{
		UpdateMenuBackground(state, delta);
	}
}

static void ControlsOptionsStateRender(GlobalState *state, const double /*delta*/)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground(state);
	}

	controlsOptionsScrollView->size.y = ScaledWindowHeightFloat() - 200;
	ProcessScrollView(controlsOptionsScrollView);
	ProcessUiStack(controlsOptionsStack);
	DrawUiStack(controlsOptionsStack);

	if (listenMode == KBM_LISTEN)
	{
		const Vector2 tooltipPos = Vector2Add(listenRow->kbmButton->anchoredPosition, v2(0, 40));
		if (listenRow->option->allowAxisBind)
		{
			RenderTooltipAt("Press a key, click a mouse button, or move the mouse wheel to bind this action.\nPress "
							"escape "
							"to unbind this action.",
							tooltipPos);
		} else
		{
			RenderTooltipAt("Press a key or click a mouse button to bind this action.\nPress escape "
							"to unbind this action.",
							tooltipPos);
		}
	} else if (listenMode == CTLR_LISTEN)
	{
		const Vector2 tooltipPos = Vector2Add(listenRow->ctlrButton->anchoredPosition, v2(0, 40));
		if (listenRow->option->allowAxisBind)
		{
			RenderTooltipAt("Click a controller button, press a trigger, or move a joystick to bind this "
							"action.\nPress start or escape to unbind this action.",
							tooltipPos);
		} else
		{
			RenderTooltipAt("Click a controller button to bind this action.\nPress start or escape to unbind this "
							"action.",
							tooltipPos);
		}
	}
}


static void ControlsOptionsStateSet()
{
	if (controlsOptionsStack == NULL)
	{
		ListInit(controlRows, LIST_POINTER);

		controlsOptionsStack = CreateUiStack();

		controlsOptionsScrollView = CreateScrollView(controlsOptionsStack, TOP_CENTER, v2(0, 100), v2(750, 200));

		int entryY = 0;
		for (size_t ci = 0; ci < controlCategories.length; ci++)
		{
			ControlCategory *cat = ListGetPointer(controlCategories, ci);
			ScrollViewAddChild(controlsOptionsScrollView,
							   CreateLabelControl(cat->categoryName,
												  16,
												  COLOR_WHITE,
												  v2(0, entryY),
												  v2(750, 40),
												  TOP_LEFT,
												  FONT_HALIGN_CENTER,
												  FONT_VALIGN_MIDDLE,
												  smallFont,
												  true));
			for (size_t i = 0; i < cat->controlOptions.length; i++)
			{
				entryY += ENTRY_HEIGHT;

				ControlOption *entry = ListGetPointer(cat->controlOptions, i);
				ScrollViewAddChild(controlsOptionsScrollView,
								   CreateLabelControl(entry->displayName,
													  16,
													  COLOR_WHITE,
													  v2(0, entryY),
													  v2(750 - 40 - 6 - 190 - 6 - 190 - 6, 40),
													  TOP_LEFT,
													  FONT_HALIGN_LEFT,
													  FONT_VALIGN_MIDDLE,
													  smallFont,
													  true));

				Control *kbmButton = CreateButtonControl(v2(0 - 40 - 6 - 190 - 6, entryY),
														 v2(190, 40),
														 "KBM",
														 BtnEditKbmBinding,
														 TOP_RIGHT,
														 NULL);
				Control *ctlrButton = CreateButtonControl(v2(0 - 40 - 6, entryY),
														  v2(190, 40),
														  "CTLR",
														  BtnEditCtlrBinding,
														  TOP_RIGHT,
														  NULL);
				Control *resetButton = CreateIconButtonControl(v2(0, entryY),
															   TEXTURE("interface/reset"),
															   BtnResetBinding,
															   TOP_RIGHT,
															   "Reset Binding");

				ControlRowData *row = malloc(sizeof(ControlRowData));
				CheckAlloc(row);
				row->option = entry;
				row->kbmButton = kbmButton;
				row->ctlrButton = ctlrButton;
				row->resetButton = resetButton;
				ListAdd(controlRows, row);

				UpdateControlRow(row);

				ScrollViewAddChild(controlsOptionsScrollView, kbmButton);
				ScrollViewAddChild(controlsOptionsScrollView, ctlrButton);
				ScrollViewAddChild(controlsOptionsScrollView, resetButton);
			}
			entryY += ENTRY_HEIGHT * 1.5f;
		}

		UiStackPush(controlsOptionsStack, CreateHeaderFooterControl(100, true, "Controls"));
		UiStackPush(controlsOptionsStack, CreateHeaderFooterControl(100, false, NULL));

		// UiStackPush(controlsOptionsStack,
		// 			CreateTextBoxControl("Filter Controls",
		// 								 v2(0, 50),
		// 								 v2(480, 40),
		// 								 TOP_CENTER,
		// 								 64,
		// 								 FilterTextChanged,
		// 								 NULL));

		UiStackPush(controlsOptionsStack,
					CreateButtonControl(v2(0, -40), v2(480, 40), "Back", BtnControlsOptionsBack, BOTTOM_CENTER, NULL));

		// UiStackPush(controlsOptionsStack,
		// 			CreateVScrollBarControl(v2(LIST_WIDTH / 2 + 20, 0),
		// 									ScaledWindowHeightFloat() - 200,
		// 									MIDDLE_CENTER,
		// 									&scrollData));
	}
	UiStackResetFocus(controlsOptionsStack);
	listenMode = NOT_LISTNENING;
}

static void ControlsOptionsStateDestroy()
{
	if (controlsOptionsStack != NULL)
	{
		FreeScrollView(controlsOptionsScrollView);
		DestroyUiStack(controlsOptionsStack);
		ListAndContentsFree(controlRows);
		controlsOptionsStack = NULL;
	}
}

const GameState ControlsOptionsState = {
	.UpdateGame = ControlsOptionsStateUpdate,
	.RenderGame = ControlsOptionsStateRender,
	.FixedUpdateGame = OptionsStateFixedUpdate,
	.Destroy = ControlsOptionsStateDestroy,
	.Set = ControlsOptionsStateSet,
	.enableRelativeMouseMode = false,
};
