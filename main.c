#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "Debug/DPrint.h"
#include "Debug/FrameBenchmark.h"
#include "Debug/FrameGrapher.h"
#include "defines.h"
#include "GameStates/GLogoSplashState.h"
#include "Helpers/CommonAssets.h"
#include "Helpers/Core/AssetReader.h"
#include "Helpers/Core/Error.h"
#include "Helpers/Core/Input.h"
#include "Helpers/Core/Logging.h"
#include "Helpers/Core/PhysicsThread.h"
#include "Helpers/Core/Timing.h"
#include "Helpers/Graphics/Drawing.h"
#include "Helpers/Graphics/RenderingHelpers.h"
#include "Helpers/PlatformHelpers.h"
#include "Structs/GlobalState.h"
#include "Structs/Level.h"
#include "Helpers/TextInputSystem.h"

SDL_Surface *windowIcon;

/**
 * Attempt to initialize the executable path
 * @param argc Program argument count
 * @param argv Program arguments
 */
void ExecPathInit(const int argc, char *argv[])
{
	if (argc < 1)
	{
		// this should *never* happen, but let's be safe
		Error("No executable path argument provided.");
	}

	if (strlen(argv[0]) > 260)
	{
		Error("Executable path too long. Please rethink your file structure.");
	}
	strncpy(GetState()->executablePath, argv[0], 260); // we do not mess around with user data in c.
	LogInfo("Executable path: %s\n", GetState()->executablePath);

	char *folder = SDL_GetBasePath();
	if (folder == NULL)
	{
		Error("Failed to get base path");
	}
	if (strlen(folder) > 260)
	{
		Error("Base path too long. Please rethink your file structure.");
	}

	strncpy(GetState()->executableFolder, folder, 260);
	SDL_free(folder);
	LogInfo("Executable folder: %s\n", GetState()->executableFolder);
}

/**
 * Initialize the SDL library
 */
void InitSDL()
{
	SDL_SetHint(SDL_HINT_APP_NAME, GAME_TITLE);
#ifdef __LINUX__
	if (GetState()->options.preferWayland)
	{
		SDL_SetHint(SDL_HINT_VIDEODRIVER, "wayland,x11"); // TODO: seems to be ignored with sdl2-compat
	} else
	{
		SDL_SetHint(SDL_HINT_VIDEODRIVER, "x11,wayland");
	}
#endif

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) != 0)
	{
		LogError("SDL_Init Error: %s\n", SDL_GetError());
		Error("Failed to initialize SDL");
	}

	LogInfo("SDL Video Driver: %s\n", SDL_GetCurrentVideoDriver());

	SDL_StopTextInput(); // is enabled by default on desktop
}

/**
 * Initialize the audio system (SDL_mixer)
 */
void InitAudio()
{
	Mix_AllocateChannels(SFX_CHANNEL_COUNT);

	if (Mix_OpenAudio(48000, AUDIO_S16, 2, 2048) == 0)
	{
		GetState()->isAudioStarted = true;
	} else
	{
		GetState()->isAudioStarted = false;
		LogError("Mix_OpenAudio Error: %s\n", Mix_GetError());
	}
}

/**
 * Initialize the game window and renderer
 */
void WindowAndRenderInit()
{
	const size_t title_len = strlen(GAME_TITLE) + strlen(" - Vulkan") + 1;
	char title[title_len];
	switch (currentRenderer) {
		case RENDERER_OPENGL:
			snprintf(title, title_len, "%s - OpenGL", GAME_TITLE);
			break;
		case RENDERER_VULKAN:
			snprintf(title, title_len, "%s - Vulkan", GAME_TITLE);
			break;
		default:
			snprintf(title, title_len, "%s", GAME_TITLE);
			break;
	}
	const Uint32 rendererFlags = currentRenderer == RENDERER_OPENGL ? SDL_WINDOW_OPENGL : SDL_WINDOW_VULKAN;
	SDL_Window *window = SDL_CreateWindow(&title[0],
										  SDL_WINDOWPOS_UNDEFINED,
										  SDL_WINDOWPOS_UNDEFINED,
										  DEF_WIDTH,
										  DEF_HEIGHT,
										  rendererFlags | SDL_WINDOW_RESIZABLE);
	if (window == NULL)
	{
		LogError("SDL_CreateWindow Error: %s\n", SDL_GetError());
		Error("Failed to create window.");
	}
	DwmDarkMode(window);
	SDL_SetWindowFullscreen(window, GetState()->options.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	SetGameWindow(window);

	if (!RenderInit())
	{
		RenderInitError();
	}

	UpdateViewportSize();

	SDL_SetWindowMinimumSize(window, MIN_WIDTH, MIN_HEIGHT);
	SDL_SetWindowMaximumSize(window, MAX_WIDTH, MAX_HEIGHT);

	windowIcon = ToSDLSurface(TEXTURE("interface_icon"), "1");
	SDL_SetWindowIcon(window, windowIcon);
}

/**
 * Handle an SDL event
 * @param event The SDL event to handle
 * @param shouldQuit Whether the program should quit after handling the event
 */
void HandleEvent(const SDL_Event event, bool *shouldQuit)
{
	switch (event.type)
	{
		case SDL_QUIT:
			*shouldQuit = true;
			break;
		case SDL_KEYUP:
			HandleKeyUp(event.key.keysym.scancode);
			break;
		case SDL_KEYDOWN:
			HandleKeyDown(event.key.keysym.scancode);
			break;
		case SDL_MOUSEMOTION:
			HandleMouseMotion(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
			break;
		case SDL_MOUSEBUTTONUP:
			HandleMouseUp(event.button.button);
			break;
		case SDL_MOUSEBUTTONDOWN:
			HandleMouseDown(event.button.button);
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				case SDL_WINDOWEVENT_MAXIMIZED:
					UpdateViewportSize();
					break;
				case SDL_WINDOWEVENT_RESTORED:
					WindowRestored();
					break;
				case SDL_WINDOWEVENT_MINIMIZED:
					WindowObscured();
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					SetLowFPS(true);
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					SetLowFPS(false);
					break;
				default:
					break;
			}
			break;
		case SDL_CONTROLLERDEVICEADDED:
			HandleControllerConnect();
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			HandleControllerDisconnect(event.cdevice.which);
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			HandleControllerButtonDown(event.cbutton.button);
			break;
		case SDL_CONTROLLERBUTTONUP:
			HandleControllerButtonUp(event.cbutton.button);
			break;
		case SDL_CONTROLLERAXISMOTION:
			HandleControllerAxis(event.caxis.axis, event.caxis.value);
			break;
		case SDL_TEXTINPUT:
			HandleTextInput(&event.text);
			break;
		default:
			break;
	}
}

int main(const int argc, char *argv[])
{
	ErrorHandlerInit();
	LogInit();
	LogInfo("Build time: %s at %s\n", __DATE__, __TIME__);
	LogInfo("Version: %s\n", VERSION);
	LogInfo("Initializing Engine\n");

	ExecPathInit(argc, argv);

	InitOptions();

	AssetCacheInit();

	InitSDL();

	PhysicsThreadInit();
	InitState();

	if (!RenderPreInit())
	{
		RenderInitError();
	}

	InitAudio();

	WindowAndRenderInit();

	InitCommonAssets();

	ChangeLevel(CreateLevel());

	GLogoSplashStateSet();

	InitTimers();

	LogInfo("Engine initialized, entering mainloop\n");

	SDL_Event event;
	bool shouldQuit = false;
	while (!shouldQuit)
	{
		while (GetState()->freezeEvents)
		{
			SDL_Delay(100);
		}
		const ulong frameStart = GetTimeNs();
#ifdef BENCHMARK_SYSTEM_ENABLE
		BenchFrameStart();
#endif

		while (SDL_PollEvent(&event) != 0)
		{
			HandleEvent(event, &shouldQuit);
		}
		GlobalState *state = GetState();

		if (!FrameStart())
		{
			if (state->UpdateGame)
			{
				state->UpdateGame(state);
			}
			if (state->requestExit)
			{
				shouldQuit = true;
			}
			if (IsLowFPSModeEnabled())
			{
				SDL_Delay(33);
			}
			continue;
		}

		ClearDepthOnly();

		ResetDPrintYPos();

		SDL_SetRelativeMouseMode(state->currentState == MAIN_STATE ? SDL_TRUE : SDL_FALSE);
		// warp the mouse to the center of the screen if we are in the main game state
		if (state->currentState == MAIN_STATE)
		{
			const Vector2 realWndSize = ActualWindowSize();
			SDL_WarpMouseInWindow(GetGameWindow(), (int)realWndSize.x / 2, (int)realWndSize.y / 2);
		}

		if (state->UpdateGame)
		{
			state->UpdateGame(state);
		}

#ifdef BENCHMARK_SYSTEM_ENABLE
		if (IsKeyJustPressed(SDL_SCANCODE_F8))
		{
			BenchToggle();
		}
#endif

		state->cam->x = state->level->player.pos.x;
		state->cam->y = (float)state->cameraY;
		state->cam->z = state->level->player.pos.y;
		state->cam->yaw = state->level->player.angle;

		state->RenderGame(state);

		FrameGraphDraw();

		FrameEnd();

		UpdateInputStates();

		if (state->requestExit)
		{
			shouldQuit = true;
		}

#ifdef BENCHMARK_SYSTEM_ENABLE
		BenchFrameEnd();
#endif

		if (IsLowFPSModeEnabled())
		{
			SDL_Delay(33);
		}
		FrameGraphUpdate(GetTimeNs() - frameStart);
	}
	LogInfo("Mainloop exited, cleaning up engine...\n");
	PhysicsThreadTerminate();
	DestroyGlobalState();
	SDL_DestroyWindow(GetGameWindow());
	SDL_FreeSurface(windowIcon);
	DestroyCommonAssets();
	DestroyAssetCache(); // Free all assets
	RenderDestroy();
	Mix_CloseAudio();
	Mix_Quit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);
	SDL_Quit();
	LogDestroy();
	return 0;
}

// Exporting these symbols tells GPU drivers to use the dedicated GPU on hybrid systems
// I do not know if these do anything on Linux, but they are here just in case.
EXPORT_SYM uint NvOptimusEnablement = 0x00000001;
EXPORT_SYM int AmdPowerXpressRequestHighPerformance = 1;
