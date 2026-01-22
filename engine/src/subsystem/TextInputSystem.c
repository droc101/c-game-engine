//
// Created by droc101 on 3/3/25.
//

#include <engine/subsystem/Logging.h>
#include <engine/subsystem/TextInputSystem.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <stddef.h>

#include "engine/graphics/RenderingHelpers.h"

TextInput *CurrentTextInput = NULL;

void SetTextInput(TextInput *input)
{
	if (CurrentTextInput != NULL)
	{
		LogError("Failed to set text input as one was already active!");
	}
	CurrentTextInput = input;
	SDL_StartTextInput(GetGameWindow());
}

void StopTextInput()
{
	if (CurrentTextInput == NULL)
	{
		LogError("Failed to stop text input as none was active!");
	}
	SDL_StopTextInput(GetGameWindow());
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
