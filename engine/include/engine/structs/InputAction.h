//
// Created by droc101 on 7/22/26.
//

/**
 * @file InputAction.h
 * Rebindable input action management functions
 */

#ifndef GAME_INPUTACTION_H
#define GAME_INPUTACTION_H

#include <engine/structs/KVList.h>
#include <engine/subsystem/Input.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stdint.h>

// forward decl. to prevent cyclic includes
typedef struct InputSystem InputSystem;

/// Types of inputs that can be bound to an input action
typedef enum InputActionBindType
{
	/// This input action is not bound
	IA_UNBOUND,
	/// This input action is bound to a keyboard key
	IA_KEY,
	/// This input action is bound to a mouse button
	IA_MOUSE_BUTTON,
	/// This input action is bound to a mouse wheel direction
	IA_MOUSE_WHEEL,
	/// This input action is bound to a controller button
	IA_CONTROLLER_BUTTON,
	/// This input action is bound to a controller axis direction
	IA_CONTROLLER_AXIS,
} InputActionBindType;

/// Control axis options
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

/// A rebindable input action
struct InputAction
{
	/// The keyboard/mouse binding type
	InputActionBindType keyboardMouseBindType;
	/// The controller binding type
	InputActionBindType controllerBindType;
	union
	{
		SDL_Scancode keyBind;
		uint8_t mouseButtonBind;
		MouseWheelAxis mouseWheelBind;
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
void LoadInputAction(const char *key, KvList list, InputAction defaults, InputAction *out);

/**
 * Save an InputAction to a KvList
 * @param key The key to write to
 * @param list The list to write into
 * @param action The action to write
 */
void SaveInputAction(const char *key, KvList list, const InputAction *action);

#endif //GAME_INPUTACTION_H
