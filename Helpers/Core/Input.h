//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "../../defines.h"

typedef enum InputState InputState;

enum InputState
{
	/// The input is not pressed
	INP_RELEASED,
	/// The input was just pressed on this frame
	INP_JUST_PRESSED,
	// / The input is currently pressed
	INP_PRESSED,
	/// The input was just released on this frame
	INP_JUST_RELEASED
};

/**
 * Handles controller disconnect event
 * @param which The controller that was disconnected
 */
void HandleControllerDisconnect(Sint32 which);

/**
 * Handles controller connect event
 */
void HandleControllerConnect();

/**
 * Handles controller button up event
 * @param button The button code
 */
void HandleControllerButtonUp(SDL_GameControllerButton button);

/**
 * Handles controller button down event
 * @param button The button code
 */
void HandleControllerButtonDown(SDL_GameControllerButton button);

/**
 * Handles controller axis event
 * @param axis The axis code
 * @param value The value of the axis
 */
void HandleControllerAxis(SDL_GameControllerAxis axis, Sint16 value);

/**
 * Handles key down event
 * @param code Key code
 */
void HandleKeyDown(int code);

/**
 * Handles key up event
 * @param code Key code
 */
void HandleKeyUp(int code);

/**
 * Handles mouse motion event
 * @param x X position
 * @param y Y position
 * @param xRel Relative X position
 * @param yRel Relative Y position
 */
void HandleMouseMotion(int x, int y, int xRel, int yRel);

/**
 * Handles mouse down event
 * @param button Button code
 */
void HandleMouseDown(int button);

/**
 * Handles mouse up event
 * @param button Button code
 */
void HandleMouseUp(int button);

/**
 * Updates input states
 * @note Only call this once per physicsFrame
 */
void UpdateInputStates();

// Exposed methods

/**
 * Checks if a controller button is pressed
 * @param button The button code
 * @return Whether the button is pressed
 */
bool IsButtonPressed(int button);

/**
 * Checks if a controller button is just pressed
 * @param button The button code
 * @return Whether the button is just pressed
 * @warning Do not use this in the physics thread, use IsButtonJustPressedPhys instead
 */
bool IsButtonJustPressed(int button);

/**
 * Checks if a controller button is just released
 * @param button The button code
 * @return Whether the button is just released
 * @warning Do not use this in the physics thread, use IsButtonJustReleasedPhys instead
 */
bool IsButtonJustReleased(int button);

/**
 * Checks if a key is pressed
 * @param code Key code
 * @return Whether the key is pressed
 */
bool IsKeyPressed(int code);

/**
 * Checks if a key is just pressed
 * @param code Key code
 * @return Whether the key is just pressed
 * @warning Do not use this in the physics thread, use IsKeyJustPressedPhys instead
 */
bool IsKeyJustPressed(int code);

/**
 * Checks if a key is just released
 * @param code Key code
 * @return Whether the key is just released
 * @warning Do not use this in the physics thread, use IsKeyJustReleasedPhys instead
 */
bool IsKeyJustReleased(int code);

/**
 * Checks if a mouse button is pressed
 * @param button Button code
 * @return Whether the button is pressed
 */
bool IsMouseButtonPressed(int button);

/**
 * Checks if a mouse button is just pressed
 * @param button Button code
 * @return Whether the button is just pressed
 * @warning Do not use this in the physics thread, use IsMouseButtonJustPressedPhys instead
 */
bool IsMouseButtonJustPressed(int button);

/**
 * Checks if a mouse button is just released
 * @param button Button code
 * @return Whether the button is just released
 * @warning Do not use this in the physics thread, use IsMouseButtonJustReleasedPhys instead
 */
bool IsMouseButtonJustReleased(int button);

/**
 * Gets the mouse position
 * @return The current mouse position
 */
Vector2 GetMousePos();

/**
 * Gets the relative mouse movement
 * @return relative mouse movement
 */
Vector2 GetMouseRel();

/**
 * Consumes a key press state, so no other input check can see it
 * @param code The key code
 */
void ConsumeKey(int code);

/**
 * Consumes a controller button press state, so no other input check can see it
 * @param btn The button code
 */
void ConsumeButton(int btn);

/**
 * Consumes a mouse button press state, so no other input check can see it
 * @param button The button code
 */
void ConsumeMouseButton(int button);

/**
 * Consumes all key press states, so no other input check can see them
 */
void ConsumeAllKeys();

/**
 * Consumes all mouse button press states, so no other input check can see them
 */
void ConsumeAllMouseButtons();

/**
 * Gets the value of a controller axis
 * @param axis The axis to get the value of
 * @return The value of the axis (between -1 and 1)
 */
float GetAxis(SDL_GameControllerAxis axis);

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
void Rumble(float strength, uint time);

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

/**
 * Swap input buffers for the physics thread
 */
void InputPhysicsFrameBegin();

/**
 * Check if a key was just pressed since the last physics tick
 * @param code The key code to check
 * @return Whether the key was just pressed
 * @warning A key can be just pressed AND just released in the same tick, so check both if you need to
 * @warning Do not use this outside of the physics thread, use IsKeyJustPressed instead
 */
bool IsKeyJustPressedPhys(int code);

/**
 * Check if a key was just released since the last physics tick
 * @param code The key code to check
 * @return Whether the key was just released
 * @warning A key can be just pressed AND just released in the same tick, so check both if you need to
 * @warning Do not use this outside of the physics thread, use IsKeyJustReleased instead
 */
bool IsKeyJustReleasedPhys(int code);

/**
 * Check if a controller button was just pressed since the last physics tick
 * @param button The button code to check
 * @return Whether the button was just pressed
 * @warning A button can be just pressed AND just released in the same tick, so check both if you need to
 * @warning Do not use this outside of the physics thread, use IsButtonJustPressed instead
 */
bool IsButtonJustPressedPhys(int button);

/**
 * Check if a controller button was just released since the last physics tick
 * @param button The button code to check
 * @return Whether the button was just released
 * @warning A button can be just pressed AND just released in the same tick, so check both if you need to
 * @warning Do not use this outside of the physics thread, use IsButtonJustReleased instead
 */
bool IsButtonJustReleasedPhys(int button);

/**
 * Check if a mouse button was just pressed since the last physics tick
 * @param button The button code to check
 * @return Whether the button was just pressed
 * @warning A button can be just pressed AND just released in the same tick, so check both if you need to
 * @warning Do not use this outside of the physics thread, use IsMouseButtonJustPressed instead
 */
bool IsMouseButtonJustPressedPhys(int button);

/**
 * Check if a mouse button was just released since the last physics tick
 * @param button The button code to check
 * @return Whether the button was just released
 * @warning A button can be just pressed AND just released in the same tick, so check both if you need to
 * @warning Do not use this outside of the physics thread, use IsMouseButtonJustReleased instead
 */
bool IsMouseButtonJustReleasedPhys(int button);

#endif //GAME_INPUT_H
