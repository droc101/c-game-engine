//
// Created by droc101 on 7/22/26.
//

#include <engine/structs/GlobalState.h>
#include <engine/structs/InputAction.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Input.h>
#include <math.h>
#include <SDL3/SDL_gamepad.h>
#include <stdbool.h>
#include <string.h>

static const char *mouseButtonNames[MAX_RECOGNIZED_MOUSE_BUTTONS] = {
	"Button 0",	 "Button 1",  "Button 2",  "Button 3",	"Button 5",	 "Button 6",  "Button 7",  "Button 8",
	"Button 9",	 "Button 10", "Button 11", "Button 12", "Button 13", "Button 14", "Button 15", "Button 16",
	"Button 17", "Button 18", "Button 19", "Button 20", "Button 21", "Button 22", "Button 23", "Button 24",
	"Button 25", "Button 26", "Button 27", "Button 28", "Button 29", "Button 30", "Button 31", "Button 32",
};

static const char *mouseWheelAxisNames[] = {
	"Wheel Up",
	"Wheel Down",
	"Wheel Left",
	"Wheel Right",
};

static const char *controllerAxisNames[] = {
	"Left Stick Up",
	"Left Stick Down",
	"Left Stick Left",
	"Left Stick Right",
	"Right Stick Up",
	"Right Stick Down",
	"Right Stick Left",
	"Right Stick Right",
	"Left Trigger",
	"Right Trigger",
};

static const char *controllerButtonNames[] = {
	"A Button",	   "B Button",	  "X Button",	   "Y Button",		 "Back",	 "Guide",	   "Start",
	"Left Stick",  "Right Stick", "Left Shoulder", "Right Shoulder", "D-Pad Up", "D-Pad Down", "D-Pad Left",
	"D-Pad Right", "Misc 1",	  "Paddle 1",	   "Paddle 2",		 "Paddle 3", "Paddle 4",   "Touchpad",
	"Misc 2",	   "Misc 3",	  "Misc 4",		   "Misc 5",		 "Misc 6",
};

static int GetMouseWheelAxisTicks(const InputSystem *system, InputActionMouseWheelAxis axis)
{
	const Vector2 wheel = GetMouseWheelTicks(system);
	switch (axis)
	{
		case MOUSE_WHEEL_UP:
			return (int)fmaxf(0.0f, wheel.y);
		case MOUSE_WHEEL_DOWN:
			return (int)fminf(0.0f, wheel.y);
		case MOUSE_WHEEL_LEFT:
			return (int)fminf(0.0f, wheel.x);
		case MOUSE_WHEEL_RIGHT:
			return (int)fmaxf(0.0f, wheel.x);
	}
	return 0;
}

static float GetMouseWheelAxis(const InputSystem *system, InputActionMouseWheelAxis axis)
{
	const Vector2 wheel = GetMouseWheel(system);
	switch (axis)
	{
		case MOUSE_WHEEL_UP:
			return fmaxf(0.0f, wheel.y);
		case MOUSE_WHEEL_DOWN:
			return fminf(0.0f, wheel.y);
		case MOUSE_WHEEL_LEFT:
			return fminf(0.0f, wheel.x);
		case MOUSE_WHEEL_RIGHT:
			return fmaxf(0.0f, wheel.x);
	}
	return 0;
}

static float GetInputActionAxis(const InputSystem *system, const InputActionControllerAxis axis)
{
	switch (axis)
	{
		case LEFT_STICK_UP:
			return fmaxf(0.0f, -GetAxis(system, SDL_GAMEPAD_AXIS_LEFTY));
		case LEFT_STICK_DOWN:
			return fmaxf(0.0f, GetAxis(system, SDL_GAMEPAD_AXIS_LEFTY));
		case LEFT_STICK_LEFT:
			return fmaxf(0.0f, -GetAxis(system, SDL_GAMEPAD_AXIS_LEFTX));
		case LEFT_STICK_RIGHT:
			return fmaxf(0.0f, GetAxis(system, SDL_GAMEPAD_AXIS_LEFTX));
		case RIGHT_STICK_UP:
			return fmaxf(0.0f, -GetAxis(system, SDL_GAMEPAD_AXIS_RIGHTY));
		case RIGHT_STICK_DOWN:
			return fmaxf(0.0f, GetAxis(system, SDL_GAMEPAD_AXIS_RIGHTY));
		case RIGHT_STICK_LEFT:
			return fmaxf(0.0f, -GetAxis(system, SDL_GAMEPAD_AXIS_RIGHTX));
		case RIGHT_STICK_RIGHT:
			return fmaxf(0.0f, GetAxis(system, SDL_GAMEPAD_AXIS_RIGHTX));
		case LEFT_TRIGGER:
			return GetAxis(system, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
		case RIGHT_TRIGGER:
			return GetAxis(system, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
	}
	return 0.0f;
}

bool IsInputActionJustPressed(const InputSystem *system, const InputAction *action)
{
	if (UseController(system))
	{
		switch (action->controllerBindType)
		{
			case IA_CONTROLLER_BUTTON:
				return IsButtonJustPressed(system, action->controllerBind.buttonBind);
			default:
				return false;
		}
	}
	switch (action->keyboardMouseBindType)
	{
		case IA_KEY:
			return IsKeyJustPressed(system, action->keyboardMouseBind.keyBind);
		case IA_MOUSE_BUTTON:
			return IsMouseButtonJustPressed(system, action->keyboardMouseBind.mouseButtonBind);
		case IA_MOUSE_WHEEL:
			return GetMouseWheelAxisTicks(system, action->keyboardMouseBind.mouseWheelBind) > 0;
		default:
			return false;
	}
}

bool IsInputActionJustReleased(const InputSystem *system, const InputAction *action)
{
	if (UseController(system))
	{
		switch (action->controllerBindType)
		{
			case IA_CONTROLLER_BUTTON:
				return IsButtonJustReleased(system, action->controllerBind.buttonBind);
			default:
				return false;
		}
	}
	switch (action->keyboardMouseBindType)
	{
		case IA_KEY:
			return IsKeyJustReleased(system, action->keyboardMouseBind.keyBind);
		case IA_MOUSE_BUTTON:
			return IsMouseButtonJustReleased(system, action->keyboardMouseBind.mouseButtonBind);
		case IA_MOUSE_WHEEL:
			return GetMouseWheelAxisTicks(system, action->keyboardMouseBind.mouseWheelBind) > 0;
		default:
			return false;
	}
}

bool IsInputActionPressed(const InputSystem *system, const InputAction *action)
{
	if (UseController(system))
	{
		switch (action->controllerBindType)
		{
			case IA_CONTROLLER_BUTTON:
				return IsButtonPressed(system, action->controllerBind.buttonBind);
			case IA_CONTROLLER_AXIS:
				return GetInputActionAxis(system, action->controllerBind.axisBind) > 0.5f;
			default:
				return false;
		}
	}
	switch (action->keyboardMouseBindType)
	{
		case IA_KEY:
			return IsKeyPressed(system, action->keyboardMouseBind.keyBind);
		case IA_MOUSE_BUTTON:
			return IsMouseButtonPressed(system, action->keyboardMouseBind.mouseButtonBind);
		case IA_MOUSE_WHEEL:
			return GetMouseWheelAxis(system, action->keyboardMouseBind.mouseWheelBind) > 0;
		default:
			return false;
	}
}

bool IsInputActionPastDeadzone(const InputSystem *system, const InputAction *action)
{
	if (UseController(system) && action->controllerBindType == IA_CONTROLLER_AXIS)
	{
		return GetInputActionAxis(system, action->controllerBind.axisBind) >= GetState()->options.controllerDeadzone;
	}
	if (action->keyboardMouseBindType == IA_MOUSE_WHEEL)
	{
		return GetMouseWheelAxisTicks(system, action->keyboardMouseBind.mouseWheelBind) > 0;
	}
	return IsInputActionPressed(system, action);
}

float InputActionGetAnalogValue(const InputSystem *system, const InputAction *action)
{
	if (UseController(system) && action->controllerBindType == IA_CONTROLLER_AXIS)
	{
		return GetInputActionAxis(system, action->controllerBind.axisBind);
	}
	if (action->keyboardMouseBindType == IA_MOUSE_WHEEL)
	{
		return GetMouseWheelAxis(system, action->keyboardMouseBind.mouseWheelBind);
	}
	return IsInputActionPressed(system, action) ? 1.0f : 0.0f;
}

const char *InputActionGetKbmString(const InputAction *action)
{
	switch (action->keyboardMouseBindType)
	{
		case IA_KEY:
			return SDL_GetScancodeName(action->keyboardMouseBind.keyBind);
		case IA_MOUSE_BUTTON:
			return mouseButtonNames[action->keyboardMouseBind.mouseButtonBind];
		case IA_MOUSE_WHEEL:
			return mouseWheelAxisNames[action->keyboardMouseBind.mouseWheelBind];
		default:
			return "(none)";
	}
}

const char *InputActionGetControllerString(const InputAction *action)
{
	switch (action->controllerBindType)
	{
		case IA_CONTROLLER_BUTTON:
			// TODO: handle different controller types (SDL_GetGamepadButtonLabel)
			return controllerButtonNames[action->controllerBind.buttonBind];
		case IA_CONTROLLER_AXIS:
			return controllerAxisNames[action->controllerBind.axisBind];
		default:
			return "(none)";
	}
}

static void CopyKbmBind(const InputAction *src, InputAction *dst)
{
	dst->keyboardMouseBindType = src->keyboardMouseBindType;
	switch (src->keyboardMouseBindType)
	{
		case IA_KEY:
			dst->keyboardMouseBind.keyBind = src->keyboardMouseBind.keyBind;
			break;
		case IA_MOUSE_BUTTON:
			dst->keyboardMouseBind.mouseButtonBind = src->keyboardMouseBind.mouseButtonBind;
			break;
		case IA_MOUSE_WHEEL:
			dst->keyboardMouseBind.mouseWheelBind = src->keyboardMouseBind.mouseWheelBind;
			break;
		default:
			break;
	}
}

static void CopyCtlBind(const InputAction *src, InputAction *dst)
{
	dst->controllerBindType = src->controllerBindType;
	switch (src->controllerBindType)
	{
		case IA_CONTROLLER_AXIS:
			dst->controllerBind.axisBind = src->controllerBind.axisBind;
			break;
		case IA_CONTROLLER_BUTTON:
			dst->controllerBind.buttonBind = src->controllerBind.buttonBind;
			break;
		default:
			break;
	}
}

void LoadInputAction(const char *key, const KvList list, const InputAction defaults, InputAction *out)
{
	if (KvHas(list, key, PARAM_TYPE_KV_LIST))
	{
		KvList actionConfig;
		if (KvGetList(list, key, actionConfig))
		{
			if (KvHas(actionConfig, "keyboard_mouse_bind_type", PARAM_TYPE_BYTE))
			{
				out->keyboardMouseBindType = KvGetByte(actionConfig, "keyboard_mouse_bind_type", IA_UNBOUND);
				switch (out->keyboardMouseBindType)
				{
					case IA_KEY:
						if (KvHas(actionConfig, "key_bind", PARAM_TYPE_INTEGER))
						{
							out->keyboardMouseBind.keyBind = KvGetInt(actionConfig, "key_bind", 0);
						} else
						{
							CopyKbmBind(&defaults, out);
						}
						break;
					case IA_MOUSE_BUTTON:
						if (KvHas(actionConfig, "mouse_button_bind", PARAM_TYPE_BYTE))
						{
							out->keyboardMouseBind.keyBind = KvGetByte(actionConfig, "mouse_button_bind", 0);
						} else
						{
							CopyKbmBind(&defaults, out);
						}
						break;
					case IA_MOUSE_WHEEL:
						if (KvHas(actionConfig, "mouse_wheel_bind", PARAM_TYPE_BYTE))
						{
							out->keyboardMouseBind.keyBind = KvGetByte(actionConfig, "mouse_wheel_bind", 0);
						} else
						{
							CopyKbmBind(&defaults, out);
						}
						break;
					default:
						out->keyboardMouseBindType = IA_UNBOUND;
						break;
				}
			} else
			{
				CopyKbmBind(&defaults, out);
			}

			if (KvHas(actionConfig, "controller_bind_type", PARAM_TYPE_BYTE))
			{
				out->controllerBindType = KvGetByte(actionConfig, "controller_bind_type", IA_UNBOUND);
				switch (out->controllerBindType)
				{
					case IA_CONTROLLER_BUTTON:
						if (KvHas(actionConfig, "controller_button_bind", PARAM_TYPE_BYTE))
						{
							out->controllerBind.buttonBind = KvGetByte(actionConfig, "controller_button_bind", 0);
						} else
						{
							CopyCtlBind(&defaults, out);
						}
						break;
					case IA_CONTROLLER_AXIS:
						if (KvHas(actionConfig, "controller_axis_bind", PARAM_TYPE_BYTE))
						{
							out->controllerBind.axisBind = KvGetByte(actionConfig, "controller_axis_bind", 0);
						} else
						{
							CopyCtlBind(&defaults, out);
						}
						break;
					default:
						out->controllerBindType = IA_UNBOUND;
						break;
				}
			} else
			{
				CopyKbmBind(&defaults, out);
			}

			KvListDestroy(actionConfig);
			return;
		}
	}

	// fallback: copy entire default action struct
	memcpy(out, &defaults, sizeof(InputAction));
}

void SaveInputAction(const char *key, const KvList list, const InputAction *action)
{
	KvList actionConfig;
	KvListCreate(actionConfig);
	KvSetByte(actionConfig, "keyboard_mouse_bind_type", action->keyboardMouseBindType);
	switch (action->keyboardMouseBindType)
	{
		case IA_KEY:
			KvSetInt(actionConfig, "key_bind", action->keyboardMouseBind.keyBind);
			break;
		case IA_MOUSE_BUTTON:
			KvSetByte(actionConfig, "mouse_button_bind", action->keyboardMouseBind.mouseButtonBind);
			break;
		case IA_MOUSE_WHEEL:
			KvSetByte(actionConfig, "mouse_wheel_bind", action->keyboardMouseBind.mouseWheelBind);
			break;
		default:
			break;
	}
	KvSetByte(actionConfig, "controller_bind_type", action->controllerBindType);
	switch (action->controllerBindType)
	{
		case IA_CONTROLLER_BUTTON:
			KvSetByte(actionConfig, "controller_button_bind", action->controllerBind.buttonBind);
			break;
		case IA_MOUSE_BUTTON:
			KvSetByte(actionConfig, "controller_axis_bind", action->controllerBind.axisBind);
			break;
		default:
			break;
	}

	KvSetList(list, key, actionConfig);
	KvListDestroy(actionConfig);
}
