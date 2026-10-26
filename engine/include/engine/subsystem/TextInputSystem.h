//
// Created by droc101 on 3/3/25.
//

/**
 * @file TextInputSystem.h
 * Text input & IME support subsystem
 */

#ifndef TEXTINPUTSYSTEM_H
#define TEXTINPUTSYSTEM_H

#include <engine/structs/Vector2.h>
#include <SDL3/SDL_events.h>
#include <stddef.h>

typedef struct TextInput TextInput;

/**
 * Callback signature for when new text is input
 * @param data The active @c TextInput
 * @param event Text input event
 */
typedef void (*TextInputCallback)(TextInput *data, SDL_TextInputEvent *event);

/**
 * Callback signature for when text is being composed with an IME
 * @param data The active @c TextInput
 * @param event Text editing event
 */
typedef void (*TextEditingCallback)(TextInput *data, SDL_TextEditingEvent *event);

/// A text input
struct TextInput
{
	/// The position of the cursor in the string. Valid range 0-strlen(text)
	size_t cursor;
	/// User data to pass to the callback, for example a pointer to the text box being edited
	void *userData;
	/// The function to call when text is input. This is only TEXT input, you must still handle special keys like backspace and arrow keys.
	TextInputCallback TextInput;
	/// The function to call when text is being composed with an IME
	TextEditingCallback TextEditing;

	/// The top-left corner of the text box
	Vector2 rectOrigin;
	/// The size of the text box
	Vector2 rectSize;
	/// The horizontal offset of the cursor in the text box
	int cursorOffsetPixels;
};

/**
 * Start a text input session
 * @param input The text input to start
 */
void SetTextInput(TextInput *input);

/// @private
void UpdateTextInputRect();

/**
 * Stop the current text input session
 */
void StopTextInput();

/**
 * Handle a text input event
 * @param event The text input event to handle
 * @private
 */
void HandleTextInput(SDL_TextInputEvent *event);

/// @private
void HandleTextEditing(SDL_TextEditingEvent *event);

#endif //TEXTINPUTSYSTEM_H
