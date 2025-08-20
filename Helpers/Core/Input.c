//
// Created by droc101 on 4/20/2024.
//

#include "Input.h"
#include <assert.h>
#include <SDL_error.h>
#include <SDL_gamecontroller.h>
#include <SDL_haptic.h>
#include <SDL_joystick.h>
#include <SDL_scancode.h>
#include <SDL_stdinc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../../Structs/GlobalState.h"
#include "../../Structs/Vector2.h"
#include "Error.h"
#include "Logging.h"

// every key is tracked, even if it's not used
// this *could* be optimized, but it's not necessary
// on modern systems where memory is not a concern
InputState keys[SDL_NUM_SCANCODES];
InputState controllerButtons[SDL_CONTROLLER_BUTTON_MAX];
InputState mouseButtons[MAX_RECOGNIZED_MOUSE_BUTTONS];

typedef struct PhysicsStateBuffer
{
	bool physKeysJustPressed[SDL_NUM_SCANCODES];
	bool physKeysJustReleased[SDL_NUM_SCANCODES];
	bool physControllerButtonsJustPressed[SDL_CONTROLLER_BUTTON_MAX];
	bool physControllerButtonsJustReleased[SDL_CONTROLLER_BUTTON_MAX];
	bool physMouseButtonsJustPressed[MAX_RECOGNIZED_MOUSE_BUTTONS];
	bool physMouseButtonsJustReleased[MAX_RECOGNIZED_MOUSE_BUTTONS];
} PhysicsStateBuffer;

/// The buffer that is actively being written to by the render thread.
PhysicsStateBuffer *physicsInputWorkingBuffer;
/// The buffer that is for the physics thread to read from.
PhysicsStateBuffer *physicsInputReadBuffer;

int mouseX;
int mouseY;
int mouseRelativeX;
int mouseRelativeY;

// 0 is left, 1 is right for axes (not 🪓)
Vector2 leftStick;
Vector2 rightStick;
Vector2 triggers;

SDL_GameController *controller;
SDL_Joystick *stick;
SDL_Haptic *haptic;

bool FindGameController()
{
	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_IsGameController(i))
		{
			controller = SDL_GameControllerOpen(i);
			stick = SDL_GameControllerGetJoystick(controller);
			if (SDL_JoystickIsHaptic(stick))
			{
				haptic = SDL_HapticOpenFromJoystick(stick);
				if (!haptic)
				{
					LogError("Failed to open haptic: %s\n",
							 SDL_GetError()); // This should never happen (if it does, SDL lied to us)
					haptic = NULL;
				} else if (SDL_HapticRumbleInit(haptic) != 0)
				{
					LogError("Failed to initialize rumble: %s\n", SDL_GetError());
					haptic = NULL;
				}
			} else
			{
				haptic = NULL;
			}
			LogInfo("Using controller \"%s\"\n", SDL_GameControllerName(controller));
			return true;
		}
	}
	return false;
}

void Rumble(const float strength, const uint32_t time)
{
	if (UseController() && haptic != NULL)
	{
		SDL_HapticRumblePlay(haptic, strength * GetState()->options.rumbleStrength, time);
	}
}

void HandleControllerDisconnect(const Sint32 which)
{
	if (controller == NULL)
	{
		return;
	}
	if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)) != which)
	{
		return;
	}
	SDL_GameControllerClose(controller);
	SDL_JoystickClose(stick);
	if (haptic)
	{
		SDL_HapticClose(haptic);
	}
	controller = NULL;
	stick = NULL;
	haptic = NULL;
	FindGameController(); // try to find another controller
}

void HandleControllerConnect()
{
	if (controller)
	{
		// disconnect the current controller to use the new one
		HandleControllerDisconnect(SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)));
	}
	FindGameController();
}

void HandleControllerButtonUp(const SDL_GameControllerButton button)
{
	controllerButtons[button] = INP_JUST_RELEASED;
	physicsInputWorkingBuffer->physControllerButtonsJustReleased[button] = true;
}

void HandleControllerButtonDown(const SDL_GameControllerButton button)
{
	controllerButtons[button] = INP_JUST_PRESSED;
	physicsInputWorkingBuffer->physControllerButtonsJustPressed[button] = true;
}

void HandleControllerAxis(const SDL_GameControllerAxis axis, const Sint16 value)
{
	const float dValue = (float)value / 32767.0f;
	switch (axis)
	{
		case SDL_CONTROLLER_AXIS_LEFTX:
			leftStick.x = dValue;
			break;
		case SDL_CONTROLLER_AXIS_LEFTY:
			leftStick.y = dValue;
			break;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			rightStick.x = dValue;
			break;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			rightStick.y = dValue;
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			triggers.x = dValue;
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			triggers.y = dValue;
			break;
		default:
			break;
	}
}

void HandleMouseMotion(const int x, const int y, const int xRel, const int yRel)
{
	mouseX = x;
	mouseY = y;
	mouseRelativeX += xRel;
	mouseRelativeY += yRel;
}

void HandleMouseDown(const int button)
{
	if (button >= MAX_RECOGNIZED_MOUSE_BUTTONS)
	{
		LogError("Received mouse event for button %d, which is greater than the maximum of %d that is supported.\n",
				 button,
				 MAX_RECOGNIZED_MOUSE_BUTTONS);
		return;
	}
	mouseButtons[button] = INP_JUST_PRESSED;
	physicsInputWorkingBuffer->physMouseButtonsJustPressed[button] = true;
}

void HandleMouseUp(const int button)
{
	if (button >= MAX_RECOGNIZED_MOUSE_BUTTONS)
	{
		LogError("Received mouse event for button %d, which is greater than the maximum of %d that is supported.\n",
				 button,
				 MAX_RECOGNIZED_MOUSE_BUTTONS);
		return;
	}
	mouseButtons[button] = INP_JUST_RELEASED;
	physicsInputWorkingBuffer->physMouseButtonsJustReleased[button] = true;
}

void HandleKeyDown(const int code)
{
	keys[code] = INP_JUST_PRESSED;
	physicsInputWorkingBuffer->physKeysJustPressed[code] = true;
}

void HandleKeyUp(const int code)
{
	keys[code] = INP_JUST_RELEASED;
	physicsInputWorkingBuffer->physKeysJustReleased[code] = true;
}

void UpdateInputStates()
{
	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		if (keys[i] == INP_JUST_RELEASED)
		{
			keys[i] = INP_RELEASED;
		} else if (keys[i] == INP_JUST_PRESSED)
		{
			keys[i] = INP_PRESSED;
		}
	}

	for (int i = 0; i < MAX_RECOGNIZED_MOUSE_BUTTONS; i++)
	{
		if (mouseButtons[i] == INP_JUST_RELEASED)
		{
			mouseButtons[i] = INP_RELEASED;
		} else if (mouseButtons[i] == INP_JUST_PRESSED)
		{
			mouseButtons[i] = INP_PRESSED;
		}
	}

	for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
	{
		if (controllerButtons[i] == INP_JUST_RELEASED)
		{
			controllerButtons[i] = INP_RELEASED;
		} else if (controllerButtons[i] == INP_JUST_PRESSED)
		{
			controllerButtons[i] = INP_PRESSED;
		}
	}

	mouseRelativeX = 0;
	mouseRelativeY = 0;
}

bool IsButtonPressed(const int button)
{
	return controllerButtons[button] == INP_PRESSED || controllerButtons[button] == INP_JUST_PRESSED;
}

bool IsButtonJustPressed(const int button)
{
	return controllerButtons[button] == INP_JUST_PRESSED;
}

bool IsButtonJustReleased(const int button)
{
	return controllerButtons[button] == INP_JUST_RELEASED;
}

bool IsKeyPressed(const int code)
{
	return keys[code] == INP_PRESSED || keys[code] == INP_JUST_PRESSED;
}

bool IsKeyJustPressed(const int code)
{
	return keys[code] == INP_JUST_PRESSED;
}

bool IsKeyJustReleased(const int code)
{
	return keys[code] == INP_JUST_RELEASED;
}

bool IsMouseButtonPressed(const int button)
{
	assert(button < MAX_RECOGNIZED_MOUSE_BUTTONS);
	return mouseButtons[button] == INP_PRESSED || mouseButtons[button] == INP_JUST_PRESSED;
}

bool IsMouseButtonJustPressed(const int button)
{
	assert(button < MAX_RECOGNIZED_MOUSE_BUTTONS);
	return mouseButtons[button] == INP_JUST_PRESSED;
}

bool IsMouseButtonJustReleased(const int button)
{
	assert(button < MAX_RECOGNIZED_MOUSE_BUTTONS);
	return mouseButtons[button] == INP_JUST_RELEASED;
}

Vector2 GetMousePos()
{
	return Vector2Scale(v2((float)mouseX, (float)mouseY), (float)(1.0 / GetState()->uiScale));
}

Vector2 GetMouseRel()
{
	return v2((float)mouseRelativeX, (float)mouseRelativeY);
}

void ConsumeKey(const int code)
{
	keys[code] = INP_RELEASED;
}

void ConsumeButton(const int btn)
{
	controllerButtons[btn] = INP_RELEASED;
}

void ConsumeMouseButton(const int button)
{
	assert(button < MAX_RECOGNIZED_MOUSE_BUTTONS);
	mouseButtons[button] = INP_RELEASED;
}

void ConsumeAllKeys()
{
	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		keys[i] = INP_RELEASED;
	}
}

void ConsumeAllMouseButtons()
{
	for (int i = 0; i < MAX_RECOGNIZED_MOUSE_BUTTONS; i++)
	{
		mouseButtons[i] = INP_RELEASED;
	}
}

float GetAxis(const SDL_GameControllerAxis axis)
{
	switch (axis)
	{
		case SDL_CONTROLLER_AXIS_LEFTX:
			return leftStick.x;
		case SDL_CONTROLLER_AXIS_LEFTY:
			return leftStick.y;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			return rightStick.x;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			return rightStick.y;
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			return triggers.x;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			return triggers.y;
		default:
			return 0;
	}
}

inline bool UseController()
{
	return GetState()->options.controllerMode && controller != NULL;
}

const char *GetControllerName()
{
	if (!UseController())
	{
		return NULL;
	}
	return SDL_GameControllerName(controller);
}

void InputInit()
{
	physicsInputReadBuffer = calloc(1, sizeof(PhysicsStateBuffer));
	CheckAlloc(physicsInputReadBuffer);
	physicsInputWorkingBuffer = calloc(1, sizeof(PhysicsStateBuffer));
	CheckAlloc(physicsInputWorkingBuffer);
}

void InputDestroy()
{
	free(physicsInputWorkingBuffer);
	free(physicsInputReadBuffer);
}

void InputPhysicsTickBegin()
{
	PhysicsStateBuffer *temp = physicsInputWorkingBuffer;
	physicsInputWorkingBuffer = physicsInputReadBuffer;
	physicsInputReadBuffer = temp;
	memset(physicsInputWorkingBuffer, 0, sizeof(PhysicsStateBuffer));
}

bool IsKeyJustPressedPhys(const int code)
{
	return physicsInputReadBuffer->physKeysJustPressed[code];
}

bool IsKeyJustReleasedPhys(const int code)
{
	return physicsInputReadBuffer->physKeysJustReleased[code];
}

bool IsButtonJustPressedPhys(const int button)
{
	return physicsInputReadBuffer->physControllerButtonsJustPressed[button];
}

bool IsButtonJustReleasedPhys(const int button)
{
	return physicsInputReadBuffer->physControllerButtonsJustReleased[button];
}

bool IsMouseButtonJustPressedPhys(const int button)
{
	return physicsInputReadBuffer->physMouseButtonsJustPressed[button];
}

bool IsMouseButtonJustReleasedPhys(const int button)
{
	return physicsInputReadBuffer->physMouseButtonsJustReleased[button];
}
