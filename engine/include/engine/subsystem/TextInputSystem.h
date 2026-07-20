//
// Created by droc101 on 3/3/25.
//

#ifndef TEXTINPUTSYSTEM_H
#define TEXTINPUTSYSTEM_H

#include <engine/structs/Vector2.h>
#include <SDL3/SDL_events.h>
#include <stddef.h>

typedef struct TextInput TextInput;

typedef void (*TextInputCallback)(TextInput *data, SDL_TextInputEvent *event);

typedef void (*TextEditingCallback)(TextInput *data, SDL_TextEditingEvent *event);

struct TextInput
{
	/// The position of the cursor in the string. Valid range 0-strlen(text)
	size_t cursor;
	/// User data to pass to the callback, for example a pointer to the text box being edited
	void *userData;
	/// The function to call when text is input. This is only TEXT input, you must still handle special keys like backspace and arrow keys.
	TextInputCallback TextInput;
	TextEditingCallback TextEditing;

	Vector2 rectOrigin;
	Vector2 rectSize;
	int cursorOffsetPixels;
};

/**
 * Start a text input session
 * @param input The text input to start
 */
void SetTextInput(TextInput *input);

void UpdateTextInputRect();

/**
 * Stop the current text input session
 */
void StopTextInput();

/**
 * Handle a text input event
 * @param event The text input event to handle
 */
void HandleTextInput(SDL_TextInputEvent *event);

void HandleTextEditing(SDL_TextEditingEvent *event);

#endif //TEXTINPUTSYSTEM_H
