//
// Created by droc101 on 3/3/25.
//

#ifndef TEXTINPUTSYSTEM_H
#define TEXTINPUTSYSTEM_H
#include <SDL_events.h>
#include <stddef.h>

typedef struct TextInput TextInput;

typedef void (*TextInputCallback)(TextInput *data, SDL_TextInputEvent *event);

struct TextInput
{
	/// The position of the cursor in the string. Valid range 0-strlen(text)
	size_t cursor;
	/// User data to pass to the callback, for example a pointer to the text box being edited
	void *userData;
	/// The function to call when text is input. This is only TEXT input, you must still handle special keys like backspace and arrow keys.
	TextInputCallback TextInput;
};

/**
 * Start a text input session
 * @param input The text input to start
 */
void SetTextInput(TextInput *input);

/**
 * Stop the current text input session
 */
void StopTextInput();

/**
 * Handle a text input event
 * @param event The text input event to handle
 */
void HandleTextInput(SDL_TextInputEvent *event);

#endif //TEXTINPUTSYSTEM_H
