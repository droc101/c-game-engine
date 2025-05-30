//
// Created by droc101 on 3/3/25.
//

#include "TextInputSystem.h"

#include "Core/Logging.h"

TextInput *CurrentTextInput = NULL;

void SetTextInput(TextInput *input)
{
	if (CurrentTextInput != NULL)
	{
		LogError("Failed to set text input as one was already active!");
	}
	CurrentTextInput = input;
	SDL_StartTextInput();
}

void StopTextInput()
{
	if (CurrentTextInput == NULL)
	{
		LogError("Failed to stop text input as none was active!");
	}
	SDL_StopTextInput();
	CurrentTextInput = NULL;
}

void HandleTextInput(SDL_TextInputEvent *event)
{
	if (CurrentTextInput == NULL)
	{
		LogError("Failed to handle text input event as no text input was active!");
		return;
	}
	CurrentTextInput->TextInput(CurrentTextInput, event);
}
