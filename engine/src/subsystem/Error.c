//
// Created by droc101 on 4/21/2024.
//

#include <engine/Engine.h>
#include <engine/graphics/gl/GLHelper.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Options.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <errno.h>
#include <SDL_error.h>
#include <SDL_messagebox.h>
#include <SDL_mixer.h>
#include <SDL_mouse.h>
#include <SDL_stdinc.h>
#include <SDL_version.h>
#include <SDL_video.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>

SDL_MessageBoxColorScheme mbColorScheme;

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
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}

	char messageBuffer[256];
	sprintf(messageBuffer, "%s\n \n%s:%d (%s)\n", error, file, line, function);

	LogError(messageBuffer);

	char messageBoxTextBuffer[768];
	sprintf(messageBoxTextBuffer,
			"Sorry, but the game has crashed.\n\n%s\n\nEngine Version: %s\nSDL Version: %d.%d.%d\nSDL_Mixer "
			"Version: "
			"%d.%d.%d\nZlib Version: %s",
			messageBuffer,
			ENGINE_VERSION,
			SDL_MAJOR_VERSION,
			SDL_MINOR_VERSION,
			SDL_PATCHLEVEL,
			SDL_MIXER_MAJOR_VERSION,
			SDL_MIXER_MINOR_VERSION,
			SDL_MIXER_PATCHLEVEL,
			ZLIB_VERSION);

	SDL_MessageBoxData mb;
	mb.message = messageBoxTextBuffer;
	mb.title = "Error";

#ifdef BUILDSTYLE_RELEASE
	const int buttonCount = 2;
#else
	const int buttonCount = 3;
#endif

	SDL_MessageBoxButtonData buttons[buttonCount];
	buttons[0].buttonid = 0;
	buttons[0].text = "Exit";
	buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
	buttons[1].buttonid = 1;
	buttons[1].text = "Restart";
	buttons[1].flags = 0;
#ifdef BUILDSTYLE_DEBUG
	buttons[2].buttonid = 2;
	buttons[2].text = "Debug";
	buttons[2].flags = 0;
#endif

	mb.buttons = buttons;
	mb.numbuttons = buttonCount;

	mb.colorScheme = &mbColorScheme;

	mb.window = GetGameWindow();
	mb.flags = SDL_MESSAGEBOX_ERROR;

	int pressedButtonID = 0;
	if (SDL_ShowMessageBox(&mb, &pressedButtonID) < 0)
	{
		LogError("Failed to show error dialog: %s\n", SDL_GetError());
	}

	switch (pressedButtonID)
	{
		case 0:
			exit(1);
		case 1:
			RestartProgram();
		case 2:
			fflush(stdout);

#ifdef WIN32
			__debugbreak(); // SIGTRAP doesn't exist on Windows
#else
			// emit sigtrap to allow debugger to catch the error
			raise(SIGTRAP);
#endif
			break;
		default:
			exit(1);
	}
	while (true)
	{}
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

void PromptRelaunch(const char *title, const char *description, const char *yesBtn, const char *noBtn)
{
	SDL_MessageBoxData mb;
	mb.message = description;
	mb.title = title;

	SDL_MessageBoxButtonData buttons[2];
	buttons[0].buttonid = 0;
	buttons[0].text = noBtn;
	buttons[0].flags = 0;
	buttons[1].buttonid = 1;
	buttons[1].text = yesBtn;
	buttons[1].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;

	mb.buttons = buttons;
	mb.numbuttons = 2;

	mb.colorScheme = &mbColorScheme;

	mb.window = GetGameWindow();
	mb.flags = SDL_MESSAGEBOX_ERROR;

	int pressedButtonID = 0;
	SDL_ShowMessageBox(&mb, &pressedButtonID);

	if (pressedButtonID == 1)
	{
		RestartProgram();
	}
}

_Noreturn void RenderInitError()
{
	LogError("Failed to initialize renderer");
	SDL_HideWindow(GetGameWindow());
	SDL_MessageBoxData mb;
	mb.title = "Failed to initialize renderer";
	if (GetState()->options.renderer == RENDERER_OPENGL)
	{
		mb.message = GL_INIT_FAIL_MSG;
	} else if (GetState()->options.renderer == RENDERER_VULKAN)
	{
		mb.message = "Failed to start the Vulkan renderer.\n"
					 "Please make sure your graphics card and drivers support Vulkan 1.2";
	}

	mb.numbuttons = 2;
	SDL_MessageBoxButtonData buttons[2];
	buttons[0].buttonid = 0;
	buttons[0].text = GetState()->options.renderer == RENDERER_OPENGL ? "Try Vulkan" : "Try OpenGL";
	buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
	buttons[1].buttonid = 1;
	buttons[1].text = "Exit";
	buttons[1].flags = 0;

	mb.colorScheme = &mbColorScheme;
	mb.buttons = buttons;
	mb.window = NULL;
	mb.flags = SDL_MESSAGEBOX_ERROR;

	int pressedButtonID = 0;
	SDL_ShowMessageBox(&mb, &pressedButtonID);
	if (pressedButtonID == 0)
	{
		if (GetState()->options.renderer == RENDERER_OPENGL)
		{
			GetState()->options.renderer = RENDERER_VULKAN;
		} else
		{
			GetState()->options.renderer = RENDERER_OPENGL;
		}
		SaveOptions(&GetState()->options);
		RestartProgram();
	} // else
	exit(1);
}

void SignalHandler(const int sig)
{
	switch (sig)
	{
		case SIGSEGV:
			Error("Segmentation Fault");
		case SIGFPE:
			Error("Floating Point Exception");
		default:
			break;
	}
}

void ErrorHandlerInit()
{
	SDL_MessageBoxColor bg;
	bg.r = 25;
	bg.g = 25;
	bg.b = 25;

	SDL_MessageBoxColor text;
	text.r = 255;
	text.g = 255;
	text.b = 255;

	SDL_MessageBoxColor buttonBorder;
	buttonBorder.r = 40;
	buttonBorder.g = 40;
	buttonBorder.b = 40;

	SDL_MessageBoxColor buttonBg;
	buttonBg.r = 35;
	buttonBg.g = 35;
	buttonBg.b = 35;

	mbColorScheme.colors[SDL_MESSAGEBOX_COLOR_BACKGROUND] = bg;
	mbColorScheme.colors[SDL_MESSAGEBOX_COLOR_TEXT] = text;
	mbColorScheme.colors[SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] = buttonBorder;
	mbColorScheme.colors[SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] = buttonBg;
	mbColorScheme.colors[SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] = text;

#ifdef BUILDSTYLE_RELEASE
	signal(SIGSEGV, SignalHandler);
	signal(SIGFPE, SignalHandler);
#endif
}

inline void TestSDLFunction_NonFatal(const int result, const char *message)
{
	if (result != 0)
	{
		LogError("%s: %s\n", message, SDL_GetError());
	}
}

inline void TestSDLFunction(const int result, const char *message, const char *userMessage)
{
	if (result != 0)
	{
		LogError("%s: %s\n", message, SDL_GetError());
		Error((char *)userMessage);
	}
}
