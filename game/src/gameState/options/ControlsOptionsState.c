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
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/InputAction.h>
#include <engine/structs/List.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/uiStack/controls/Button.h>
#include <engine/uiStack/controls/TextBox.h>
#include <engine/uiStack/controls/VScrollBar.h>
#include <engine/uiStack/UiStack.h>
#include <gameState/OptionsState.h>
#include <math.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "gameState/options/InputOptionsState.h"

typedef struct ControlOption
{
	char name[64];
	InputAction *action;
	bool allowAxisBind;
	const InputAction *defaultAction;
} ControlOption;

typedef enum ListenMode
{
	NOT_LISTNENING,
	KBM_LISTEN,
	CTLR_LISTEN,
} ListenMode;

static List controlOptions = {0};

static UiStack *controlsOptionsStack = NULL;
static char *filter = NULL;

static const int LIST_WIDTH = 750;
static const int ENTRY_HEIGHT = 44;

static ListenMode listenMode;
static size_t listenIndex;

static VScrollBarData scrollData = {0};

static void BtnControlsOptionsBack()
{
	SetGameState(&InputOptionsState);
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
		InputAction *action = ((ControlOption *)ListGetPointer(controlOptions, listenIndex))->action;
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
				return;
			}

			const uint8_t pressedMouseButton = GetPressedMouseButton(mainThreadInput);
			if (pressedMouseButton != 0xFF)
			{
				action->keyboardMouseBindType = IA_MOUSE_BUTTON;
				action->keyboardMouseBind.mouseButtonBind = pressedMouseButton;
				listenMode = NOT_LISTNENING;
				ConsumeMouseButton(mainThreadInput, pressedMouseButton);
				return;
			}

			if (((ControlOption *)ListGetPointer(controlOptions, listenIndex))->allowAxisBind)
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
				return;
			}

			if (((ControlOption *)ListGetPointer(controlOptions, listenIndex))->allowAxisBind)
			{
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > 0.0f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = LEFT_TRIGGER;
					listenMode = NOT_LISTNENING;
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.0f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = RIGHT_TRIGGER;
					listenMode = NOT_LISTNENING;
					return;
				}

				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_LEFTY) > 0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = LEFT_STICK_DOWN;
					listenMode = NOT_LISTNENING;
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_LEFTY) < -0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = LEFT_STICK_UP;
					listenMode = NOT_LISTNENING;
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_LEFTX) > 0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = LEFT_STICK_RIGHT;
					listenMode = NOT_LISTNENING;
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_LEFTX) < -0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = LEFT_STICK_LEFT;
					listenMode = NOT_LISTNENING;
					return;
				}

				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_RIGHTY) > 0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = RIGHT_STICK_DOWN;
					listenMode = NOT_LISTNENING;
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_RIGHTY) < -0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = RIGHT_STICK_UP;
					listenMode = NOT_LISTNENING;
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_RIGHTX) > 0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = RIGHT_STICK_RIGHT;
					listenMode = NOT_LISTNENING;
					return;
				}
				if (GetAxis(mainThreadInput, SDL_GAMEPAD_AXIS_RIGHTX) < -0.5f)
				{
					action->controllerBindType = IA_CONTROLLER_AXIS;
					action->controllerBind.axisBind = RIGHT_STICK_LEFT;
					listenMode = NOT_LISTNENING;
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
		BtnControlsOptionsBack();
	}
	if (!optionsStateInGame)
	{
		UpdateMenuBackground(state, delta);
	}
}

static bool IsRectHovered(Vector2 pos, Vector2 size)
{
	const Vector2 endPos = Vector2Add(pos, size);
	const Vector2 mousePos = GetMousePos(mainThreadInput);

	if (mousePos.y < 100 || mousePos.y > ScaledWindowHeightFloat() - 100)
	{
		return false;
	}

	return mousePos.x >= pos.x && mousePos.x <= endPos.x && mousePos.y > pos.y && mousePos.y <= endPos.y;
}

static void ControlsOptionsStateRender(GlobalState *state, const double /*delta*/)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground(state);
		DrawRect(0, 0, ScaledWindowWidth(), ScaledWindowHeight(), COLOR(0x40000000));
	}

	const int listX = ScaledWindowWidth() / 2 - LIST_WIDTH / 2;

	Vector2 tooltipPos = v2s(0);
	bool resetTooltip = false;

	int entryY = 102 + scrollData.scrollPos;
	int numShownEntries = 0;
	for (size_t i = 0; i < controlOptions.length; i++)
	{
		ControlOption *entry = ListGetPointer(controlOptions, i);

		if (entry->action != NULL && filter != NULL && strcasestr(entry->name, filter) == NULL)
		{
			continue;
		}

		if (entry->action)
		{
			DrawTextAligned(entry->name,
							20,
							COLOR_WHITE,
							v2(listX, entryY),
							v2(LIST_WIDTH, ENTRY_HEIGHT),
							FONT_HALIGN_LEFT,
							FONT_VALIGN_MIDDLE,
							smallFont);

			const Vector2 buttonSize = v2(190, ENTRY_HEIGHT - 4);
			const Vector2 resetButtonSize = v2(ENTRY_HEIGHT - 4, ENTRY_HEIGHT - 4);
			const Vector2
					leftButtonPos = v2(listX + LIST_WIDTH - buttonSize.x - 4 - buttonSize.x - 4 - resetButtonSize.x,
									   entryY + 2);
			const Vector2 rightButtonPos = v2(listX + LIST_WIDTH - buttonSize.x - 4 - resetButtonSize.x, entryY + 2);
			const Vector2 resetButtonPos = v2(listX + LIST_WIDTH - resetButtonSize.x, entryY + 2);

			const char *leftButtonTexture = TEXTURE("interface/button");
			if (listenMode == KBM_LISTEN && listenIndex == i)
			{
				leftButtonTexture = TEXTURE("interface/button_pressed");
				tooltipPos = Vector2Add(leftButtonPos, v2(0, 40));
			} else if (IsRectHovered(leftButtonPos, buttonSize))
			{
				leftButtonTexture = TEXTURE("interface/button_hover");
				if (IsMouseButtonPressed(mainThreadInput, SDL_BUTTON_LEFT))
				{
					leftButtonTexture = TEXTURE("interface/button_pressed");
				}
				if (IsMouseButtonJustPressed(mainThreadInput, SDL_BUTTON_LEFT))
				{
					listenMode = KBM_LISTEN;
					listenIndex = i;
					ConsumeMouseButton(mainThreadInput, SDL_BUTTON_LEFT);
					UiStackResetFocus(controlsOptionsStack);
					(void)PlaySound(SOUND("sfx/click"), SOUND_CATEGORY_UI);
				}
			}

			const char *rightButtonTexture = TEXTURE("interface/button");
			if (listenMode == CTLR_LISTEN && listenIndex == i)
			{
				rightButtonTexture = TEXTURE("interface/button_pressed");
				tooltipPos = Vector2Add(rightButtonPos, v2(0, 40));
			} else if (IsRectHovered(rightButtonPos, buttonSize))
			{
				rightButtonTexture = TEXTURE("interface/button_hover");
				if (IsMouseButtonPressed(mainThreadInput, SDL_BUTTON_LEFT))
				{
					rightButtonTexture = TEXTURE("interface/button_pressed");
				}
				if (IsMouseButtonJustPressed(mainThreadInput, SDL_BUTTON_LEFT))
				{
					listenMode = CTLR_LISTEN;
					listenIndex = i;
					ConsumeMouseButton(mainThreadInput, SDL_BUTTON_LEFT);
					UiStackResetFocus(controlsOptionsStack);
					(void)PlaySound(SOUND("sfx/click"), SOUND_CATEGORY_UI);
				}
			}

			const char *resetButtonTexture = TEXTURE("interface/button");
			if (IsRectHovered(resetButtonPos, resetButtonSize))
			{
				resetTooltip = true;
				resetButtonTexture = TEXTURE("interface/button_hover");
				if (IsMouseButtonPressed(mainThreadInput, SDL_BUTTON_LEFT))
				{
					resetButtonTexture = TEXTURE("interface/button_pressed");
				}
				if (IsMouseButtonJustReleased(mainThreadInput, SDL_BUTTON_LEFT))
				{
					listenMode = NOT_LISTNENING;
					memcpy(entry->action, entry->defaultAction, sizeof(InputAction));
					ConsumeMouseButton(mainThreadInput, SDL_BUTTON_LEFT);
					UiStackResetFocus(controlsOptionsStack);
					(void)PlaySound(SOUND("sfx/click"), SOUND_CATEGORY_UI);
				}
			}

			DrawNinePatchTexture(resetButtonPos, resetButtonSize, 8, 8, resetButtonTexture);
			DrawTexture(resetButtonPos, resetButtonSize, TEXTURE("interface/reset"));

			DrawNinePatchTexture(leftButtonPos, buttonSize, 8, 8, leftButtonTexture);
			if (listenMode == KBM_LISTEN && listenIndex == i)
			{
				DrawTextAligned("...",
								16,
								COLOR_BLACK,
								leftButtonPos,
								buttonSize,
								FONT_HALIGN_CENTER,
								FONT_VALIGN_MIDDLE,
								smallFont);
			} else
			{
				const char *leftButtonLabel = InputActionGetKbmString(entry->action);
				DrawTextAligned(leftButtonLabel,
								16,
								COLOR_BLACK,
								leftButtonPos,
								buttonSize,
								FONT_HALIGN_CENTER,
								FONT_VALIGN_MIDDLE,
								smallFont);
			}

			DrawNinePatchTexture(rightButtonPos, buttonSize, 8, 8, rightButtonTexture);
			const char *rightButtonLabel = InputActionGetControllerString(entry->action);
			if (listenMode == CTLR_LISTEN && listenIndex == i)
			{
				DrawTextAligned("...",
								16,
								COLOR_BLACK,
								rightButtonPos,
								buttonSize,
								FONT_HALIGN_CENTER,
								FONT_VALIGN_MIDDLE,
								smallFont);
			} else
			{
				DrawTextAligned(rightButtonLabel,
								16,
								COLOR_BLACK,
								rightButtonPos,
								buttonSize,
								FONT_HALIGN_CENTER,
								FONT_VALIGN_MIDDLE,
								smallFont);
			}
		} else
		{
			DrawTextAligned(entry->name,
							16,
							COLOR_WHITE,
							v2(listX, entryY),
							v2(LIST_WIDTH, ENTRY_HEIGHT),
							FONT_HALIGN_CENTER,
							FONT_VALIGN_MIDDLE,
							smallFont);
		}

		entryY += ENTRY_HEIGHT + 2;
		numShownEntries++;
	}

	DrawRect(0, 0, ScaledWindowWidth(), 100, COLOR(0x80000000));
	DrawRect(0, ScaledWindowHeight() - 100, ScaledWindowWidth(), 100, COLOR(0x80000000));
	DrawLine(v2(0, 99), v2(ScaledWindowWidth(), 99), 2, COLOR_WHITE);
	DrawLine(v2(0, ScaledWindowHeight() - 99), v2(ScaledWindowWidth(), ScaledWindowHeight() - 99), 2, COLOR_WHITE);

	DrawTextAligned("Controls",
					32,
					COLOR_WHITE,
					v2s(0),
					v2(ScaledWindowWidthFloat(), 50),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					largeFont);

	scrollData.contentHeight = numShownEntries * (ENTRY_HEIGHT + 2);
	scrollData.scrollPos += (int)GetMouseWheelTicks(mainThreadInput).y * ENTRY_HEIGHT / 2;

	ProcessUiStack(controlsOptionsStack);
	DrawUiStack(controlsOptionsStack);

	if (listenMode == KBM_LISTEN)
	{
		ControlOption *option = ListGetPointer(controlOptions, listenIndex);
		if (option->allowAxisBind)
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
		ControlOption *option = ListGetPointer(controlOptions, listenIndex);
		if (option->allowAxisBind)
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

	if (resetTooltip)
	{
		RenderTooltipAtMouse("Reset Binding");
	}
}

static ControlOption *CreateControlOption(const char *name,
										  InputAction *action,
										  const bool allowAxisBind,
										  const InputAction *defaultAction)
{
	ControlOption *option = malloc(sizeof(ControlOption));
	CheckAlloc(option);
	strncpy(option->name, name, sizeof(option->name));
	option->action = action;
	option->allowAxisBind = allowAxisBind;
	option->defaultAction = defaultAction;
	return option;
}

static void ControlsOptionsStateSet()
{
	if (controlOptions.data == NULL)
	{
		ListInit(controlOptions, LIST_POINTER);
		ListAdd(controlOptions, CreateControlOption("Movement", NULL, false, NULL));
		ListAdd(controlOptions,
				CreateControlOption("Move Forward", &GetState()->options.moveForward, true, &DEFAULT_MOVE_FORWARD));
		ListAdd(controlOptions,
				CreateControlOption("Move Backward", &GetState()->options.moveBackward, true, &DEFAULT_MOVE_BACKWARD));
		ListAdd(controlOptions,
				CreateControlOption("Move Left", &GetState()->options.moveLeft, true, &DEFAULT_MOVE_LEFT));
		ListAdd(controlOptions,
				CreateControlOption("Move Right", &GetState()->options.moveRight, true, &DEFAULT_MOVE_RIGHT));
		ListAdd(controlOptions,
				CreateControlOption("Sprint (only in noclip rn)", &GetState()->options.sprint, true, &DEFAULT_SPRINT));
		ListAdd(controlOptions, CreateControlOption("Sneak", &GetState()->options.sneak, true, &DEFAULT_SNEAK));
		ListAdd(controlOptions, CreateControlOption("Jump", &GetState()->options.jump, false, &DEFAULT_JUMP));

		ListAdd(controlOptions, CreateControlOption("Interaction", NULL, false, NULL));
		ListAdd(controlOptions,
				CreateControlOption("Interact", &GetState()->options.interact, false, &DEFAULT_INTERACT));
		ListAdd(controlOptions,
				CreateControlOption("Primary Attack",
									&GetState()->options.primaryAttack,
									false,
									&DEFAULT_PRIMARY_ATTACK));
		ListAdd(controlOptions,
				CreateControlOption("Secondary Attack",
									&GetState()->options.secondaryAttack,
									false,
									&DEFAULT_SECONDARY_ATTACK));
		ListAdd(controlOptions,
				CreateControlOption("Previous Item", &GetState()->options.previousItem, true, &DEFAULT_PREVIOUS_ITEM));
		ListAdd(controlOptions,
				CreateControlOption("Next Item", &GetState()->options.nextItem, true, &DEFAULT_NEXT_ITEM));

		ListAdd(controlOptions, CreateControlOption("Camera", NULL, false, NULL));
		ListAdd(controlOptions, CreateControlOption("Look Up", &GetState()->options.lookUp, true, &DEFAULT_LOOK_UP));
		ListAdd(controlOptions,
				CreateControlOption("Look Down", &GetState()->options.lookDown, true, &DEFAULT_LOOK_DOWN));
		ListAdd(controlOptions,
				CreateControlOption("Look Left", &GetState()->options.lookLeft, true, &DEFAULT_LOOK_LEFT));
		ListAdd(controlOptions,
				CreateControlOption("Look Right", &GetState()->options.lookRight, true, &DEFAULT_LOOK_RIGHT));

		ListAdd(controlOptions, CreateControlOption("Debug", NULL, false, NULL));
		ListAdd(controlOptions,
				CreateControlOption("Toggle Debug Menu", &GetState()->options.debugMenu, false, &DEFAULT_DEBUG_MENU));
		ListAdd(controlOptions,
				CreateControlOption("Toggle Noclip", &GetState()->options.noclip, false, &DEFAULT_NOCLIP));
		ListAdd(controlOptions,
				CreateControlOption("Toggle Freecam", &GetState()->options.freecam, false, &DEFAULT_FREECAM));
		ListAdd(controlOptions,
				CreateControlOption("Start/Stop Benchmark", &GetState()->options.benchmark, false, &DEFAULT_BENCHMARK));
	}

	if (controlsOptionsStack == NULL)
	{
		controlsOptionsStack = CreateUiStack();

		UiStackPush(controlsOptionsStack,
					CreateTextBoxControl("Filter Controls",
										 v2(0, 50),
										 v2(480, 40),
										 TOP_CENTER,
										 64,
										 FilterTextChanged,
										 NULL));

		UiStackPush(controlsOptionsStack,
					CreateButtonControl(v2(0, -40), v2(480, 40), "Back", BtnControlsOptionsBack, BOTTOM_CENTER, NULL));

		UiStackPush(controlsOptionsStack,
					CreateVScrollBarControl(v2(LIST_WIDTH / 2 + 20, 0),
											ScaledWindowHeightFloat() - 200,
											MIDDLE_CENTER,
											&scrollData));
	}
	UiStackResetFocus(controlsOptionsStack);
	listenMode = NOT_LISTNENING;
}

static void ControlsOptionsStateDestroy()
{
	if (controlsOptionsStack != NULL)
	{
		DestroyUiStack(controlsOptionsStack);
		controlsOptionsStack = NULL;
	}
	if (controlOptions.length != 0)
	{
		ListAndContentsFree(controlOptions);
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
