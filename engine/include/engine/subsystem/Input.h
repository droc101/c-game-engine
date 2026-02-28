//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include <engine/structs/GlobalState.h> // NOLINT(*-include-cleaner)
#include <engine/structs/Vector2.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_joystick.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct InputSystem InputSystem;

extern InputSystem *physicsThreadInput;
extern InputSystem *mainThreadInput;

#define STICK_DEADZONE 0.1

#define MAX_RECOGNIZED_MOUSE_BUTTONS 32 // *surely* nobody will have a mouse with 33 or more buttons...right?

/// Use this for the "OK/Accept" button in place of hardcoding controller A or B buttons
#define CONTROLLER_OK (GetState()->options.controllerSwapOkCancel ? SDL_GAMEPAD_BUTTON_EAST : SDL_GAMEPAD_BUTTON_SOUTH)
/// Use this for the "Cancel" button in place of hardcoding controller A or B buttons
#define CONTROLLER_CANCEL \
	(GetState()->options.controllerSwapOkCancel ? SDL_GAMEPAD_BUTTON_SOUTH : SDL_GAMEPAD_BUTTON_EAST)

/**
 * Handles controller disconnect event
 * @param which The controller that was disconnected
 */
void HandleGamepadDisconnect(SDL_JoystickID which);

/**
 * Handles controller connect event
 */
void HandleGamepadConnect();

/**
 * Process an input event
 * @param system The input system to modify
 * @param event The event to process
 * @return Whether the event was processed
 */
bool InputSystemProcessEvent(InputSystem *system, const SDL_Event *event);

/**
 * Updates input states
 */
void UpdateInputStates(InputSystem *system);

// Exposed methods

/**
 * Checks if a controller button is pressed
 * @param system The input system to check
 * @param button The button code
 * @return Whether the button is pressed
 */
bool IsButtonPressed(const InputSystem *system, int button);

/**
 * Checks if a controller button is just pressed
 * @param system The input system to check
 * @param button The button code
 * @return Whether the button is just pressed
 * @warning Do not use this in the physics thread, use IsButtonJustPressedPhys instead
 */
bool IsButtonJustPressed(const InputSystem *system, int button);

/**
 * Checks if a controller button is just released
 * @param system The input system to check
 * @param button The button code
 * @return Whether the button is just released
 * @warning Do not use this in the physics thread, use IsButtonJustReleasedPhys instead
 */
bool IsButtonJustReleased(const InputSystem *system, int button);

/**
 * Checks if a key is pressed
 * @param system The input system to check
 * @param code Key code
 * @return Whether the key is pressed
 */
bool IsKeyPressed(const InputSystem *system, int code);

/**
 * Checks if a key is just pressed
 * @param system The input system to check
 * @param code Key code
 * @return Whether the key is just pressed
 * @warning Do not use this in the physics thread, use IsKeyJustPressedPhys instead
 */
bool IsKeyJustPressed(const InputSystem *system, int code);

/**
 * Checks if a key is just released
 * @param system The input system to check
 * @param code Key code
 * @return Whether the key is just released
 * @warning Do not use this in the physics thread, use IsKeyJustReleasedPhys instead
 */
bool IsKeyJustReleased(const InputSystem *system, int code);

/**
 * Checks if a mouse button is pressed
 * @param system The input system to check
 * @param button Button code
 * @return Whether the button is pressed
 */
bool IsMouseButtonPressed(const InputSystem *system, int button);

/**
 * Checks if a mouse button is just pressed
 * @param system The input system to check
 * @param button Button code
 * @return Whether the button is just pressed
 * @warning Do not use this in the physics thread, use IsMouseButtonJustPressedPhys instead
 */
bool IsMouseButtonJustPressed(const InputSystem *system, int button);

/**
 * Checks if a mouse button is just released
 * @param system The input system to check
 * @param button Button code
 * @return Whether the button is just released
 * @warning Do not use this in the physics thread, use IsMouseButtonJustReleasedPhys instead
 */
bool IsMouseButtonJustReleased(const InputSystem *system, int button);

/**
 * Gets the mouse position
 * @param system The input system to check
 * @return The current mouse position
 */
Vector2 GetMousePos(const InputSystem *system);

/**
 * Gets the relative mouse movement
 * @param system The input system to check
 * @return relative mouse movement
 */
Vector2 GetMouseRel(const InputSystem *system);

/**
 * Get the relative mouse wheel movement
 * @param system The input system to check
 * @return relative mouse wheel movement
 */
Vector2 GetMouseWheel(const InputSystem *system);

/**
 * Consumes a key press state, so no other input check can see it
 * @param system The input system to modify
 * @param code The key code
 */
void ConsumeKey(InputSystem *system, int code);

/**
 * Consumes a controller button press state, so no other input check can see it
 * @param system The input system to modify
 * @param btn The button code
 */
void ConsumeButton(InputSystem *system, int btn);

/**
 * Consumes a mouse button press state, so no other input check can see it
 * @param system The input system to modify
 * @param button The button code
 */
void ConsumeMouseButton(InputSystem *system, int button);

/**
 * Consumes all key press states, so no other input check can see them
 * @param system The input system to modify
 */
void ConsumeAllKeys(InputSystem *system);

/**
 * Consumes all mouse button press states, so no other input check can see them
 * @param system The input system to modify
 */
void ConsumeAllMouseButtons(InputSystem *system);

/**
 * Gets the value of a controller axis
 * @param system The input system to check
 * @param axis The axis to get the value of
 * @return The value of the axis (between -1 and 1)
 */
float GetAxis(const InputSystem *system, SDL_GamepadAxis axis);

/**
 * Checks if a controller is being used
 * @return whether a controller is being used
 */
bool UseController();

/**
 * Rumble the controller (if available)
 * @param strength The base strength of the rumble (0.0 - 1.0)
 * @param time The time to rumble in milliseconds
 */
void Rumble(float strength, uint32_t time);

/**
 * Get the name of the connected controller
 * @return The name of the connected controller, or NULL if no controller is connected
 */
const char *GetControllerName();

/**
 * Initializes the input system
 */
void InputInit();

/**
 * Destroys the input system
 */
void InputDestroy();

#endif //GAME_INPUT_H
