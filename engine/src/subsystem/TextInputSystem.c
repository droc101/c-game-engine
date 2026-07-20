//
// Created by droc101 on 3/3/25.
//

#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/TextInputSystem.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_rect.h>
#include <stddef.h>

static TextInput *CurrentTextInput = NULL;

void SetTextInput(TextInput *input)
{
	if (CurrentTextInput != NULL)
	{
		LogError("Failed to set text input as one was already active!\n");
	}
	CurrentTextInput = input;
	SDL_StartTextInput(GetGameWindow());
}

void UpdateTextInputRect()
{
	if (CurrentTextInput != NULL)
	{
		const SDL_Rect rect = {
			(int)(CurrentTextInput->rectOrigin.x * GetState()->uiScale),
			(int)(CurrentTextInput->rectOrigin.y * GetState()->uiScale),
			(int)(CurrentTextInput->rectSize.x * GetState()->uiScale),
			(int)(CurrentTextInput->rectSize.y * GetState()->uiScale),
		};
		SDL_SetTextInputArea(GetGameWindow(), &rect, CurrentTextInput->cursorOffsetPixels);
	}
}

void StopTextInput()
{
	if (CurrentTextInput == NULL)
	{
		LogError("Failed to stop text input as none was active!\n");
	}
	SDL_StopTextInput(GetGameWindow());
	CurrentTextInput = NULL;
}

void HandleTextInput(SDL_TextInputEvent *event)
{
	if (CurrentTextInput == NULL)
	{
		LogError("Failed to handle text input event as no text input was active!\n");
		return;
	}
	CurrentTextInput->TextInput(CurrentTextInput, event);
}

void HandleTextEditing(SDL_TextEditingEvent *event)
{
	if (CurrentTextInput == NULL)
	{
		LogError("Failed to handle text editing event as no text input was active!");
		return;
	}
	if (CurrentTextInput->TextEditing)
	{
		CurrentTextInput->TextEditing(CurrentTextInput, event);
	}
}
