//
// Created by droc101 on 4/21/2024.
//

#include <engine/Engine.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <errno.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_messagebox.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool signalHandlerInvoked = false;

_Noreturn inline void _GameAllocFailure()
{
	LogError("Memory Allocation Failed: %s\n", strerror(errno));
	if (errno == ENOMEM) // NOLINT(*-include-cleaner)
	{
		exit(1); // We should not attempt to do complex things if we are out of memory
	}
	Error("Memory Allocation Failed");
}

_Noreturn void _ErrorInternal(char *error, const char *file, const int line, const char *function)
{
	if (GetGameWindow() != NULL)
	{
		GetState()->freezeEvents = true;
		SDL_SetWindowRelativeMouseMode(GetGameWindow(), false);
	}

	LogError("Fatal Error: %s\n", error);
	LogError("At: %s:%d (%s)\n", file, line, function);

	PrintStackTrace();

	char messageBoxTextBuffer[1024];
#ifdef BUILDSTYLE_DEBUG
	snprintf(messageBoxTextBuffer,
			 1024,
			 "%s\n\nAt: %s:%d (%s)\nEngine Version: %s",
			 error,
			 file,
			 line,
			 function,
			 ENGINE_VERSION);
#else
	snprintf(messageBoxTextBuffer, 1024, "%s", error);
#endif

	SDL_MessageBoxData mb;
	mb.message = messageBoxTextBuffer;
	mb.title = "Fatal Error";

#ifdef BUILDSTYLE_RELEASE
	const int buttonCount = 3;
#else
	const int buttonCount = 4;
#endif

	SDL_MessageBoxButtonData buttons[buttonCount];
	buttons[0].buttonID = 0;
	buttons[0].text = "Exit";
	buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
	buttons[1].buttonID = 1;
	buttons[1].text = "Restart";
	buttons[1].flags = 0;
	buttons[2].buttonID = 2;
	buttons[2].text = "View Logs";
	buttons[2].flags = 0;
#ifdef BUILDSTYLE_DEBUG
	buttons[3].buttonID = 3;
	buttons[3].text = "Debug";
	buttons[3].flags = 0;
#endif

	mb.buttons = buttons;
	mb.numbuttons = buttonCount;

	mb.window = GetGameWindow();
	mb.flags = SDL_MESSAGEBOX_ERROR;

	int pressedButtonID = 0;
	if (!SDL_ShowMessageBox(&mb, &pressedButtonID))
	{
		LogError("Failed to show error dialog: %s\n", SDL_GetError());
	}

	switch (pressedButtonID)
	{
		case 1:
			RestartProgram();
			break;
		case 2:
			LogDestroy();
			OpenFileInDefaultProgram("game.log");
			break;
		case 3:
			fflush(stdout);
			RaiseDebugger();
			break;
		default:
			break;
	}
	exit(1);
}

_Noreturn void FriendlyError(const char *title, const char *description)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, description, NULL);
	exit(1);
}

void ShowWarning(const char *title, const char *description)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, title, description, NULL);
}

_Noreturn void RenderInitError()
{
	LogError("Failed to initialize renderer\n");
	SDL_HideWindow(GetGameWindow());
	SDL_MessageBoxData mb;
	mb.title = "Failed to initialize renderer";
	mb.message = "Failed to start the Vulkan renderer.\n"
				 "Please make sure your graphics card and drivers support Vulkan 1.2";

	mb.numbuttons = 1;
	SDL_MessageBoxButtonData buttons[1];
	buttons[0].buttonID = 1;
	buttons[0].text = "Exit";
	buttons[0].flags = 0;

	mb.buttons = buttons;
	mb.window = NULL;
	mb.flags = SDL_MESSAGEBOX_ERROR;

	int pressedButtonID = 0;
	SDL_ShowMessageBox(&mb, &pressedButtonID);
	exit(1);
}

static void SignalHandler(const int sig)
{
	if (signalHandlerInvoked)
	{
		return;
	}
	signalHandlerInvoked = true;
	switch (sig)
	{
		case SIGSEGV:
			Error("Segmentation Fault");
		case SIGFPE:
			Error("Floating Point Exception");
		case SIGILL:
			Error("Illegal Instruction");
		case SIGABRT:
			Error("Abort");
		default:
			break;
	}
}

void ErrorHandlerInit()
{
#ifdef BUILDSTYLE_RELEASE
	signal(SIGSEGV, SignalHandler);
	signal(SIGFPE, SignalHandler);
	signal(SIGILL, SignalHandler);
	signal(SIGABRT, SignalHandler);
#endif
}
