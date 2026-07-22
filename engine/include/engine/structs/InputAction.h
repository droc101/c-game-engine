//
// Created by droc101 on 7/22/26.
//

#ifndef GAME_INPUTACTION_H
#define GAME_INPUTACTION_H

#include <engine/structs/KVList.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stdint.h>

// forward decl. to prevent cyclic includes
typedef struct InputSystem InputSystem;

typedef enum InputActionBindType
{
	IA_UNBOUND,
	IA_KEY,
	IA_MOUSE_BUTTON,
	IA_MOUSE_WHEEL,
	IA_CONTROLLER_BUTTON,
	IA_CONTROLLER_AXIS,
} InputActionBindType;

typedef enum InputActionMouseWheelAxis
{
	MOUSE_WHEEL_UP,
	MOUSE_WHEEL_DOWN,
	MOUSE_WHEEL_LEFT,
	MOUSE_WHEEL_RIGHT,
} InputActionMouseWheelAxis;

typedef enum InputActionControllerAxis
{
	LEFT_STICK_UP,
	LEFT_STICK_DOWN,
	LEFT_STICK_LEFT,
	LEFT_STICK_RIGHT,
	RIGHT_STICK_UP,
	RIGHT_STICK_DOWN,
	RIGHT_STICK_LEFT,
	RIGHT_STICK_RIGHT,
	LEFT_TRIGGER,
	RIGHT_TRIGGER,
} InputActionControllerAxis;

typedef struct InputAction InputAction;

struct InputAction
{
	InputActionBindType keyboardMouseBindType;
	InputActionBindType controllerBindType;
	union
	{
		SDL_Scancode keyBind;
		uint8_t mouseButtonBind;
		InputActionMouseWheelAxis mouseWheelBind;
	} keyboardMouseBind;
	union
	{
		SDL_GamepadButton buttonBind;
		InputActionControllerAxis axisBind;
	} controllerBind;
};

/**
 * Check if an input action was just pressed
 * @param system The input system to check
 * @param action The action to check
 */
bool IsInputActionJustPressed(const InputSystem *system, const InputAction *action);

/**
 * Check if an input action was just released
 * @param system The input system to check
 * @param action The action to check
 */
bool IsInputActionJustReleased(const InputSystem *system, const InputAction *action);

/**
 * Check if an input action is pressed
 * @param system The input system to check
 * @param action The action to check
 */
bool IsInputActionPressed(const InputSystem *system, const InputAction *action);

/**
 * Check if an input action is past the deadzone (for digital inputs, this is the same as @c IsInputActionPressed)
 * @param system The input system to check
 * @param action The action to check
 */
bool IsInputActionPastDeadzone(const InputSystem *system, const InputAction *action);

/**
 * Get the 0...1 analog value of an input action (for digital actions, it will return 0 or 1)
 * @param system The input system to check
 * @param action The action to check
 */
float InputActionGetAnalogValue(const InputSystem *system, const InputAction *action);

/**
 * Get the display label of an input action's keyboard/mouse binding
 */
const char *InputActionGetKbmString(const InputAction *action);

/**
 * Get the display label of an input action's controller binding
 */
const char *InputActionGetControllerString(const InputAction *action);

/**
 * Load an InputAction from a KvList
 * @param key The key to look for
 * @param list The list to look in
 * @param defaults Default values in case parts of or all of loading fails
 * @param out The InputAction to load into
 */
void LoadInputAction(const char *key, const KvList list, InputAction defaults, InputAction *out);

/**
 * Save an InputAction to a KvList
 * @param key The key to write to
 * @param list The list to write into
 * @param action The action to write
 */
void SaveInputAction(const char *key, const KvList list, const InputAction *action);

#endif //GAME_INPUTACTION_H
