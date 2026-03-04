//
// Created by droc101 on 4/20/2024.
//

#include <assert.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/Logging.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum InputState : uint8_t
{
	/// The input is not pressed
	INP_RELEASED,
	/// The input was just pressed on this frame
	INP_JUST_PRESSED,
	/// The input is currently pressed
	INP_PRESSED,
	/// The input was just released on this frame
	INP_JUST_RELEASED,
} InputState;

struct InputSystem
{
	// every key is tracked, even if it's not used
	// this *could* be optimized, but it's not necessary
	// on modern systems where memory is not a concern
	InputState keys[SDL_SCANCODE_COUNT];
	bool queueReleaseKeys[SDL_SCANCODE_COUNT];

	InputState controllerButtons[SDL_GAMEPAD_BUTTON_COUNT];
	bool queueReleaseControllerButtons[SDL_GAMEPAD_BUTTON_COUNT];
	float gamepadAxes[SDL_GAMEPAD_AXIS_COUNT];

	InputState mouseButtons[MAX_RECOGNIZED_MOUSE_BUTTONS];
	bool queueReleaseMouseButtons[MAX_RECOGNIZED_MOUSE_BUTTONS];
	int mouseX;
	int mouseY;
	int mouseRelativeX;
	int mouseRelativeY;

	float mouseWheelRelativeX;
	float mouseWheelRelativeY;
	int mouseWheelRelativeTicksX;
	int mouseWheelRelativeTicksY;
};

SDL_Gamepad *currentGamepad;
SDL_Joystick *currentJoystick;
bool gamepadHasBasicHaptics;
bool gamepadHasTriggerHaptics;

InputSystem *physicsThreadInput = NULL;
InputSystem *mainThreadInput = NULL;

bool FindGamepad()
{
	int numGamepads = 0;
	SDL_JoystickID *gamepads = SDL_GetGamepads(&numGamepads);
	for (int i = 0; i < numGamepads; i++)
	{
		const SDL_JoystickID gamepad = gamepads[i];
		if (SDL_IsGamepad(gamepad))
		{
			currentGamepad = SDL_OpenGamepad(gamepad);
			currentJoystick = SDL_GetGamepadJoystick(currentGamepad);
			const SDL_PropertiesID gamepadProps = SDL_GetGamepadProperties(currentGamepad);
			gamepadHasBasicHaptics = SDL_GetBooleanProperty(gamepadProps, SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false);
			gamepadHasTriggerHaptics = SDL_GetBooleanProperty(gamepadProps,
															  SDL_PROP_GAMEPAD_CAP_TRIGGER_RUMBLE_BOOLEAN,
															  false);

			LogInfo("Using controller \"%s\"\n", SDL_GetGamepadName(currentGamepad));
			LogDebug("Controller features basic haptics: %s\n", gamepadHasBasicHaptics ? "yes" : "no");
			LogDebug("Controller features trigger haptics: %s\n", gamepadHasTriggerHaptics ? "yes" : "no");

			SDL_free(gamepads);
			return true;
		}
	}
	SDL_free(gamepads);
	return false;
}

void Rumble(const float strength, const uint32_t time)
{
	if (UseController() && gamepadHasBasicHaptics)
	{
		const uint16_t uintStrength = (uint16_t)((strength * GetState()->options.rumbleStrength) * UINT16_MAX);
		SDL_RumbleGamepad(currentGamepad, uintStrength, uintStrength, time);
	}
}

void HandleGamepadDisconnect(const SDL_JoystickID which)
{
	if (currentGamepad == NULL)
	{
		return;
	}
	if (SDL_GetJoystickID(SDL_GetGamepadJoystick(currentGamepad)) != which)
	{
		return;
	}
	SDL_CloseGamepad(currentGamepad);
	SDL_CloseJoystick(currentJoystick);
	currentGamepad = NULL;
	currentJoystick = NULL;
	FindGamepad(); // try to find another controller
}

void HandleGamepadConnect()
{
	if (currentGamepad)
	{
		// disconnect the current controller to use the new one
		HandleGamepadDisconnect(SDL_GetJoystickID(SDL_GetGamepadJoystick(currentGamepad)));
	}
	FindGamepad();
}

void UpdateInputState(InputState *statePtr, bool *queueReleasePtr, const InputState newState)
{
	if (*statePtr == INP_JUST_PRESSED && newState == INP_JUST_RELEASED)
	{
		*queueReleasePtr = true;
	} else
	{
		*statePtr = newState;
		*queueReleasePtr = false;
	}
}

bool InputSystemProcessEvent(InputSystem *system, const SDL_Event *event)
{
	switch (event->type)
	{
		case SDL_EVENT_KEY_UP:
			if (!event->key.repeat)
			{
				UpdateInputState(&system->keys[event->key.scancode],
								 &system->queueReleaseKeys[event->key.scancode],
								 INP_JUST_RELEASED);
			}
			break;
		case SDL_EVENT_KEY_DOWN:
			if (!event->key.repeat)
			{
				UpdateInputState(&system->keys[event->key.scancode],
								 &system->queueReleaseKeys[event->key.scancode],
								 INP_JUST_PRESSED);
			}
			break;
		case SDL_EVENT_MOUSE_MOTION:
			system->mouseX = (int)event->motion.x;
			system->mouseY = (int)event->motion.y;
			system->mouseRelativeX += (int)event->motion.xrel;
			system->mouseRelativeY += (int)event->motion.yrel;
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (event->button.button >= MAX_RECOGNIZED_MOUSE_BUTTONS)
			{
				LogError("Received mouse event for button %d, which is greater than the maximum of %d that is "
						 "supported.\n",
						 event->button.button,
						 MAX_RECOGNIZED_MOUSE_BUTTONS);
				break;
			}
			UpdateInputState(&system->mouseButtons[event->button.button],
							 &system->queueReleaseMouseButtons[event->button.button],
							 INP_JUST_RELEASED);
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (event->button.button >= MAX_RECOGNIZED_MOUSE_BUTTONS)
			{
				LogError("Received mouse event for button %d, which is greater than the maximum of %d that is "
						 "supported.\n",
						 event->button.button,
						 MAX_RECOGNIZED_MOUSE_BUTTONS);
				break;
			}
			UpdateInputState(&system->mouseButtons[event->button.button],
							 &system->queueReleaseMouseButtons[event->button.button],
							 INP_JUST_PRESSED);
			break;
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			UpdateInputState(&system->controllerButtons[event->gbutton.button],
							 &system->queueReleaseControllerButtons[event->gbutton.button],
							 INP_JUST_PRESSED);
			break;
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			UpdateInputState(&system->controllerButtons[event->gbutton.button],
							 &system->queueReleaseControllerButtons[event->gbutton.button],
							 INP_JUST_RELEASED);
			break;
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			system->gamepadAxes[event->gaxis.axis] = (float)event->gaxis.value / 32767.0f;
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			system->mouseWheelRelativeX += event->wheel.x;
			system->mouseWheelRelativeY += event->wheel.y;
			system->mouseWheelRelativeTicksX += event->wheel.integer_x;
			system->mouseWheelRelativeTicksY += event->wheel.integer_y;
			break;
		default:
			return false;
	}
	return true;
}

void UpdateInputStates(InputSystem *system)
{
	for (int i = 0; i < SDL_SCANCODE_COUNT; i++)
	{
		if (system->queueReleaseKeys[i])
		{
			system->keys[i] = INP_JUST_RELEASED;
			system->queueReleaseKeys[i] = false;
		} else if (system->keys[i] == INP_JUST_RELEASED)
		{
			system->keys[i] = INP_RELEASED;
		} else if (system->keys[i] == INP_JUST_PRESSED)
		{
			system->keys[i] = INP_PRESSED;
		}
	}

	for (int i = 0; i < MAX_RECOGNIZED_MOUSE_BUTTONS; i++)
	{
		if (system->queueReleaseMouseButtons[i])
		{
			system->mouseButtons[i] = INP_JUST_RELEASED;
			system->queueReleaseMouseButtons[i] = false;
		} else if (system->mouseButtons[i] == INP_JUST_RELEASED)
		{
			system->mouseButtons[i] = INP_RELEASED;
		} else if (system->mouseButtons[i] == INP_JUST_PRESSED)
		{
			system->mouseButtons[i] = INP_PRESSED;
		}
	}

	for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; i++)
	{
		if (system->queueReleaseControllerButtons[i])
		{
			system->controllerButtons[i] = INP_JUST_RELEASED;
			system->queueReleaseControllerButtons[i] = false;
		} else if (system->controllerButtons[i] == INP_JUST_RELEASED)
		{
			system->controllerButtons[i] = INP_RELEASED;
		} else if (system->controllerButtons[i] == INP_JUST_PRESSED)
		{
			system->controllerButtons[i] = INP_PRESSED;
		}
	}

	system->mouseRelativeX = 0;
	system->mouseRelativeY = 0;
	system->mouseWheelRelativeX = 0;
	system->mouseWheelRelativeY = 0;
	system->mouseWheelRelativeTicksX = 0;
	system->mouseWheelRelativeTicksY = 0;
}

bool IsButtonPressed(const InputSystem *system, const int button)
{
	return system->controllerButtons[button] == INP_PRESSED || system->controllerButtons[button] == INP_JUST_PRESSED;
}

bool IsButtonJustPressed(const InputSystem *system, const int button)
{
	return system->controllerButtons[button] == INP_JUST_PRESSED;
}

bool IsButtonJustReleased(const InputSystem *system, const int button)
{
	return system->controllerButtons[button] == INP_JUST_RELEASED;
}

bool IsKeyPressed(const InputSystem *system, const int code)
{
	return system->keys[code] == INP_PRESSED || system->keys[code] == INP_JUST_PRESSED;
}

bool IsKeyJustPressed(const InputSystem *system, const int code)
{
	return system->keys[code] == INP_JUST_PRESSED;
}

bool IsKeyJustReleased(const InputSystem *system, const int code)
{
	return system->keys[code] == INP_JUST_RELEASED;
}

bool IsMouseButtonPressed(const InputSystem *system, const int button)
{
	assert(button < MAX_RECOGNIZED_MOUSE_BUTTONS);
	return system->mouseButtons[button] == INP_PRESSED || system->mouseButtons[button] == INP_JUST_PRESSED;
}

bool IsMouseButtonJustPressed(const InputSystem *system, const int button)
{
	assert(button < MAX_RECOGNIZED_MOUSE_BUTTONS);
	return system->mouseButtons[button] == INP_JUST_PRESSED;
}

bool IsMouseButtonJustReleased(const InputSystem *system, const int button)
{
	assert(button < MAX_RECOGNIZED_MOUSE_BUTTONS);
	return system->mouseButtons[button] == INP_JUST_RELEASED;
}

Vector2 GetMousePos(const InputSystem *system)
{
	return Vector2Scale(v2((float)system->mouseX, (float)system->mouseY), (float)(1.0 / GetState()->uiScale));
}

Vector2 GetMouseRel(const InputSystem *system)
{
	return v2((float)system->mouseRelativeX, (float)system->mouseRelativeY);
}

Vector2 GetMouseWheel(const InputSystem *system)
{
	return v2(system->mouseWheelRelativeX, system->mouseWheelRelativeY);
}

Vector2 GetMouseWheelTicks(const InputSystem *system)
{
	return v2(system->mouseWheelRelativeTicksX, system->mouseWheelRelativeTicksY);
}

void ConsumeKey(InputSystem *system, const int code)
{
	system->keys[code] = INP_RELEASED;
}

void ConsumeButton(InputSystem *system, const int btn)
{
	system->controllerButtons[btn] = INP_RELEASED;
}

void ConsumeMouseButton(InputSystem *system, const int button)
{
	assert(button < MAX_RECOGNIZED_MOUSE_BUTTONS);
	system->mouseButtons[button] = INP_RELEASED;
}

void ConsumeAllKeys(InputSystem *system)
{
	for (int i = 0; i < SDL_SCANCODE_COUNT; i++)
	{
		system->keys[i] = INP_RELEASED;
	}
}

void ConsumeAllMouseButtons(InputSystem *system)
{
	for (int i = 0; i < MAX_RECOGNIZED_MOUSE_BUTTONS; i++)
	{
		system->mouseButtons[i] = INP_RELEASED;
	}
}

float GetAxis(const InputSystem *system, const SDL_GamepadAxis axis)
{
	return system->gamepadAxes[axis];
}

inline bool UseController()
{
	return GetState()->options.controllerMode && currentGamepad != NULL;
}

const char *GetControllerName()
{
	if (!UseController())
	{
		return NULL;
	}
	return SDL_GetGamepadName(currentGamepad);
}

void InputInit()
{
	LogDebug("Initializing input system...\n");
	mainThreadInput = calloc(1, sizeof(InputSystem));
	CheckAlloc(mainThreadInput);
	physicsThreadInput = calloc(1, sizeof(InputSystem));
	CheckAlloc(physicsThreadInput);
}

void InputDestroy()
{
	LogDebug("Cleaning up input system...\n");
	free(mainThreadInput);
	free(physicsThreadInput);
}
