//
// Created by droc101 on 4/20/2024.
//

#include <assert.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/Logging.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_haptic.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum InputState
{
	/// The input is not pressed
	INP_RELEASED,
	/// The input was just pressed on this frame
	INP_JUST_PRESSED,
	/// The input is currently pressed
	INP_PRESSED,
	/// The input was just released on this frame
	INP_JUST_RELEASED
} InputState;

typedef struct PhysicsStateBuffer
{
	bool physKeysJustPressed[SDL_SCANCODE_COUNT];
	bool physKeysJustReleased[SDL_SCANCODE_COUNT];
	bool physControllerButtonsJustPressed[SDL_GAMEPAD_BUTTON_COUNT];
	bool physControllerButtonsJustReleased[SDL_GAMEPAD_BUTTON_COUNT];
	bool physMouseButtonsJustPressed[MAX_RECOGNIZED_MOUSE_BUTTONS];
	bool physMouseButtonsJustReleased[MAX_RECOGNIZED_MOUSE_BUTTONS];
} PhysicsStateBuffer;

// every key is tracked, even if it's not used
// this *could* be optimized, but it's not necessary
// on modern systems where memory is not a concern
static InputState keys[SDL_SCANCODE_COUNT];
static InputState controllerButtons[SDL_GAMEPAD_BUTTON_COUNT];
static InputState mouseButtons[MAX_RECOGNIZED_MOUSE_BUTTONS];

/// The buffer that is actively being written to by the render thread.
static PhysicsStateBuffer *physicsInputWorkingBuffer;
/// The buffer that is for the physics thread to read from.
static PhysicsStateBuffer *physicsInputReadBuffer;

static int mouseX;
static int mouseY;
static int mouseRelativeX;
static int mouseRelativeY;

// 0 is left, 1 is right for axes (not 🪓)
static Vector2 leftStick;
static Vector2 rightStick;
static Vector2 triggers;

static SDL_Gamepad *controller;
static SDL_Joystick *stick;
static SDL_Haptic *haptic;

bool FindGameController()
{
	int numGamepads = 0;
	SDL_JoystickID *gamepads = SDL_GetGamepads(&numGamepads);
	for (int i = 0; i < numGamepads; i++)
	{
		const SDL_JoystickID gamepad = gamepads[i];
		if (SDL_IsGamepad(gamepad))
		{
			controller = SDL_OpenGamepad(gamepad);
			stick = SDL_GetGamepadJoystick(controller);
			if (SDL_IsJoystickHaptic(stick)) // TODO my controller HAS haptics WHY is this false
			{
				haptic = SDL_OpenHapticFromJoystick(stick);
				if (!haptic)
				{
					LogError("Failed to open haptic: %s\n",
							 SDL_GetError()); // This should never happen (if it does, SDL lied to us)
					haptic = NULL;
				} else if (!SDL_InitHapticRumble(haptic))
				{
					LogError("Failed to initialize rumble: %s\n", SDL_GetError());
					haptic = NULL;
				}
			} else
			{
				haptic = NULL;
			}
			LogInfo("Using controller \"%s\"\n", SDL_GetGamepadName(controller));
			SDL_free(gamepads);
			return true;
		}
	}
	SDL_free(gamepads);
	return false;
}

void Rumble(const float strength, const uint32_t time)
{
	if (UseController() && haptic != NULL)
	{
		SDL_PlayHapticRumble(haptic, strength * GetState()->options.rumbleStrength, time);
	}
}

void HandleControllerDisconnect(const SDL_JoystickID which)
{
	if (controller == NULL)
	{
		return;
	}
	if (SDL_GetJoystickID(SDL_GetGamepadJoystick(controller)) != which)
	{
		return;
	}
	SDL_CloseGamepad(controller);
	SDL_CloseJoystick(stick);
	if (haptic)
	{
		SDL_CloseHaptic(haptic);
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
		HandleControllerDisconnect(SDL_GetJoystickID(SDL_GetGamepadJoystick(controller)));
	}
	FindGameController();
}

void HandleControllerButtonUp(const SDL_GamepadButton button)
{
	controllerButtons[button] = INP_JUST_RELEASED;
	physicsInputWorkingBuffer->physControllerButtonsJustReleased[button] = true;
}

void HandleControllerButtonDown(const SDL_GamepadButton button)
{
	controllerButtons[button] = INP_JUST_PRESSED;
	physicsInputWorkingBuffer->physControllerButtonsJustPressed[button] = true;
}

void HandleControllerAxis(const SDL_GamepadAxis axis, const Sint16 value)
{
	const float dValue = (float)value / 32767.0f;
	switch (axis)
	{
		case SDL_GAMEPAD_AXIS_LEFTX:
			leftStick.x = dValue;
			break;
		case SDL_GAMEPAD_AXIS_LEFTY:
			leftStick.y = dValue;
			break;
		case SDL_GAMEPAD_AXIS_RIGHTX:
			rightStick.x = dValue;
			break;
		case SDL_GAMEPAD_AXIS_RIGHTY:
			rightStick.y = dValue;
			break;
		case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
			triggers.x = dValue;
			break;
		case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
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
	for (int i = 0; i < SDL_SCANCODE_COUNT; i++)
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

	for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; i++)
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
	for (int i = 0; i < SDL_SCANCODE_COUNT; i++)
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

float GetAxis(const SDL_GamepadAxis axis)
{
	switch (axis)
	{
		case SDL_GAMEPAD_AXIS_LEFTX:
			return leftStick.x;
		case SDL_GAMEPAD_AXIS_LEFTY:
			return leftStick.y;
		case SDL_GAMEPAD_AXIS_RIGHTX:
			return rightStick.x;
		case SDL_GAMEPAD_AXIS_RIGHTY:
			return rightStick.y;
		case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
			return triggers.x;
		case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
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
	return SDL_GetGamepadName(controller);
}

void InputInit()
{
	LogDebug("Initializing input system...\n");
	physicsInputReadBuffer = calloc(1, sizeof(PhysicsStateBuffer));
	CheckAlloc(physicsInputReadBuffer);
	physicsInputWorkingBuffer = calloc(1, sizeof(PhysicsStateBuffer));
	CheckAlloc(physicsInputWorkingBuffer);
}

void InputDestroy()
{
	LogDebug("Cleaning up input system...\n");
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
