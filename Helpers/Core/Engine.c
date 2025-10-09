//
// Created by droc101 on 10/7/2025.
//

#include "Engine.h"
#include <SDL.h>
#include <SDL_cpuinfo.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_filesystem.h>
#include <SDL_hints.h>
#include <SDL_keyboard.h>
#include <SDL_mixer.h>
#include <SDL_mouse.h>
#include <SDL_scancode.h>
#include <SDL_stdinc.h>
#include <SDL_surface.h>
#include <SDL_timer.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../../config.h"
#include "../../Debug/DPrint.h"
#include "../../Debug/FrameBenchmark.h"
#include "../../Debug/FrameGrapher.h"
#include "../../GameStates/GLogoSplashState.h"
#include "../../GameStates/GMainState.h"
#include "../../GameStates/GMenuState.h"
#include "../../Structs/ActorDefinition.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Vector2.h"
#include "../Discord.h"
#include "../Graphics/Drawing.h"
#include "../Graphics/Font.h"
#include "../Graphics/LodThread.h"
#include "../Graphics/RenderingHelpers.h"
#include "../PlatformHelpers.h"
#include "../TextInputSystem.h"
#include "Arguments.h"
#include "AssetLoaders/GameConfigLoader.h"
#include "AssetReader.h"
#include "Error.h"
#include "Input.h"
#include "Logging.h"
#include "Physics/Physics.h"
#include "Physics/PhysicsThread.h"
#include "SoundSystem.h"
#include "Timing.h"

SDL_Surface *windowIcon;
SDL_Event event;
bool shouldQuit = false;

bool CheckCPUSupport()
{
	// TODO i sure hope this doesn't rely on any of the extensions
	LogInfo("System has %d processors and %d MiB RAM.\n", SDL_GetCPUCount(), SDL_GetSystemRAM());
	bool cpuSupported = true;
	if (SDL_GetCPUCount() < 4)
	{
		LogWarning("Running on system with less than 4 threads, this may be very slow!\n");
	}
#ifdef __x86_64__
	if (!SDL_HasMMX())
	{
		LogError("CPU does not have the MMX extension!\n");
		cpuSupported = false;
	}
	if (!SDL_HasSSE())
	{
		LogError("CPU does not have the SSE extension!\n");
		cpuSupported = false;
	}
	if (!SDL_HasSSE2())
	{
		LogError("CPU does not have the SSE2 extension!\n");
		cpuSupported = false;
	}
	// TODO SSE4(.0)
	if (!SDL_HasSSE41())
	{
		LogError("CPU does not have the SSE4.1 extension!\n");
		cpuSupported = false;
	}
	if (!SDL_HasSSE42())
	{
		LogError("CPU does not have the SSE4.2 extension!\n");
		cpuSupported = false;
	}
	if (!SDL_HasAVX())
	{
		LogError("CPU does not have the AVX extension!\n");
		cpuSupported = false;
	}
	if (!SDL_HasAVX2())
	{
		LogError("CPU does not have the AVX2 extension!\n");
		cpuSupported = false;
	}
#endif
	return cpuSupported;
}

void ExecPathInit(const int argc, const char *argv[])
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

void InitSDL()
{
	LogDebug("Initializing SDL...\n");
	SDL_SetHint(SDL_HINT_APP_NAME, config.gameTitle);
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

void WindowAndRenderInit()
{
	LogDebug("Creating window...\n");
	const size_t titleLen = strlen(config.gameTitle) + strlen(" - Vulkan") + 1;
	char title[titleLen];
	switch (currentRenderer)
	{
		case RENDERER_OPENGL:
			snprintf(title, titleLen, "%s - OpenGL", config.gameTitle);
			break;
		case RENDERER_VULKAN:
			snprintf(title, titleLen, "%s - Vulkan", config.gameTitle);
			break;
		default:
			snprintf(title, titleLen, "%s", config.gameTitle);
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
	SetDwmWindowAttribs(window);
	SDL_SetWindowFullscreen(window, GetState()->options.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	SetGameWindow(window);

	if (!RenderInit())
	{
		RenderInitError();
	}

	UpdateViewportSize();

	SDL_SetWindowMinimumSize(window, MIN_WIDTH, MIN_HEIGHT);
	SDL_SetWindowMaximumSize(window, MAX_WIDTH, MAX_HEIGHT);

	LogDebug("Setting window icon...\n");
	windowIcon = ToSDLSurface(TEXTURE("interface/icon"), "1");
	SDL_SetWindowIcon(window, windowIcon);
}

void HandleEvent(void)
{
	switch (event.type)
	{
		case SDL_QUIT:
			shouldQuit = true;
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

void InitEngine(const int argc, const char *argv[])
{
	ErrorHandlerInit();
	ExecPathInit(argc, argv);
	LogInit();
	LogInfo("Build time: %s at %s\n", __DATE__, __TIME__);
	LogInfo("Engine Version: %s\n", ENGINE_VERSION);
	LogInfo("Initializing Engine\n");

	if (!CheckCPUSupport())
	{
		Error("Your computer's CPU does not meet the minimum requirements.");
	}

	LoadGameConfig();

	InitOptions();

	InitTimers();

	if (HasCliArg(argc, argv, "--renderer"))
	{
		const char *renderer = GetCliArgStr(argc, argv, "--renderer", "gl");
		if (strncmp(renderer, "gl", strlen("gl")) == 0)
		{
			LogInfo("Forcing OpenGL Renderer\n");
			GetState()->options.renderer = RENDERER_OPENGL;
		} else if (strncmp(renderer, "vulkan", strlen("vulkan")) == 0)
		{
			LogInfo("Forcing Vulkan Renderer\n");
			GetState()->options.renderer = RENDERER_VULKAN;
		}
	}

	PhysicsInitGlobal(GetState());

	AssetCacheInit();

	InitSDL();

	InputInit();

	RegisterActors();

	InitState();
	PhysicsThreadInit();

	if (!RenderPreInit())
	{
		RenderInitError();
	}

	InitSoundSystem();

	WindowAndRenderInit();

	InitCommonFonts();

	bool loadMap = false;
	if (HasCliArg(argc, argv, "--map"))
	{
		const char *mapName = GetCliArgStr(argc, argv, "--map", "");
		if (ChangeLevelByName(mapName))
		{
			loadMap = true;
		}
	}
	if (loadMap)
	{
		GMainStateSet();
	} else
	{
		if (!HasCliArg(argc, argv, "--nosplash"))
		{
			GLogoSplashStateSet();
		} else
		{
			GMenuStateSet();
		}
	}

	DiscordInit();
}

void EngineIteration()
{
	while (GetState()->freezeEvents)
	{
		SDL_Delay(100);
	}
	const uint64_t frameStart = GetTimeNs();
#ifdef BENCHMARK_SYSTEM_ENABLE
	BenchFrameStart();
#endif

	while (SDL_PollEvent(&event) != 0)
	{
		HandleEvent();
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
		return;
	}

	ClearScreen();

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

	if (state->level)
	{
		// TODO should this be moved somewhere else?
		state->camera->transform.position.x = state->level->player.transform.position.x;
		state->camera->transform.position.y = state->level->player.transform.position.y; // + state->camera->yOffset;
		state->camera->transform.position.z = state->level->player.transform.position.z;
		state->camera->transform.rotation = state->level->player.transform.rotation;
		state->viewmodel.transform.position.y = state->camera->yOffset * 0.2f - 0.35f;
	}

	state->RenderGame(state);

	FrameGraphDraw();
	TickGraphDraw();

	FrameEnd();

	UpdateInputStates();

	DiscordUpdate();

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

void DestroyEngine()
{
	DiscordDestroy();
	PhysicsThreadTerminate();
	LodThreadDestroy();
	InputDestroy();
	DestroyGlobalState();
	DestroySoundSystem();
	RenderDestroy();
	LogDebug("Cleaning up window...\n");
	SDL_DestroyWindow(GetGameWindow());
	LogDebug("Cleaning up icon...\n");
	SDL_FreeSurface(windowIcon);
	DestroyCommonFonts();
	DestroyAssetCache(); // Free all assets
	LogDebug("Cleaning up SDL_Mixer...\n");
	Mix_CloseAudio();
	Mix_Quit();
	LogDebug("Cleaning up SDL...\n");
	SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);
	SDL_Quit();
	LogDestroy();
}

bool EngineShouldQuit()
{
	return shouldQuit;
}
