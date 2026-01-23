//
// Created by droc101 on 10/7/2025.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/debug/DPrint.h>
#include <engine/debug/FrameBenchmark.h>
#include <engine/debug/FrameGrapher.h>
#include <engine/Engine.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/Arguments.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/physics/Physics.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/subsystem/TextInputSystem.h>
#include <engine/subsystem/threads/LodThread.h>
#include <engine/subsystem/threads/PhysicsThread.h>
#include <engine/subsystem/Timing.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Exporting these symbols tells GPU drivers to use the dedicated GPU on hybrid systems
// I do not know if these do anything on Linux, but they are here just in case.
EXPORT_SYM uint32_t NvOptimusEnablement = 0x00000001;
EXPORT_SYM int AmdPowerXpressRequestHighPerformance = 1;

SDL_Surface *windowIcon;
SDL_Event event;
bool shouldQuit = false;

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

	const char *folder = SDL_GetBasePath();
	if (folder == NULL)
	{
		Error("Failed to get base path");
	}
	if (strlen(folder) > 260)
	{
		Error("Base path too long. Please rethink your file structure.");
	}

	strncpy(GetState()->executableFolder, folder, 260);
	// SDL_free(folder); // TODO needed?
	LogInfo("Executable folder: %s\n", GetState()->executableFolder);
}

void InitSDL()
{
	LogDebug("Initializing SDL...\n");
	SDL_SetHint(SDL_HINT_APP_NAME, gameConfig.gameTitle);
#ifdef SDL_PLATFORM_LINUX
	if (GetState()->options.preferWayland)
	{
		SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland,x11");
	} else
	{
		SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11,wayland");
	}
#endif

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC))
	{
		LogError("SDL_Init Error: %s\n", SDL_GetError());
		Error("Failed to initialize SDL");
	}

	LogInfo("SDL Video Driver: %s\n", SDL_GetCurrentVideoDriver());
}

void WindowAndRenderInit()
{
	LogDebug("Creating window...\n");
	const size_t titleLen = strlen(gameConfig.gameTitle) + strlen(" - Vulkan") + 1;
	char title[titleLen];
	switch (currentRenderer)
	{
		case RENDERER_OPENGL:
			snprintf(title, titleLen, "%s - OpenGL", gameConfig.gameTitle);
			break;
		case RENDERER_VULKAN:
			snprintf(title, titleLen, "%s - Vulkan", gameConfig.gameTitle);
			break;
		default:
			snprintf(title, titleLen, "%s", gameConfig.gameTitle);
			break;
	}
	SDL_SetHint(SDL_HINT_VIDEO_FORCE_EGL, "1"); // TODO: GLEW won't init (error 1) with GLX
	const Uint32 rendererFlags = currentRenderer == RENDERER_OPENGL ? SDL_WINDOW_OPENGL : SDL_WINDOW_VULKAN;
	SDL_Window *window = SDL_CreateWindow(&title[0],
										  DEF_WIDTH,
										  DEF_HEIGHT,
										  rendererFlags | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	if (window == NULL)
	{
		LogError("SDL_CreateWindow Error: %s\n", SDL_GetError());
		Error("Failed to create window.");
	}
	SetDwmWindowAttribs(window);
	SDL_SetWindowFullscreen(window, GetState()->options.fullscreen); // TODO exclusive vs not?
	SDL_StopTextInput(window);
	SetGameWindow(window);

	if (!RenderInit())
	{
		RenderInitError();
	}

	UpdateViewportSize();

	SDL_SetWindowMinimumSize(window, MIN_WIDTH, MIN_HEIGHT);
	SDL_SetWindowMaximumSize(window, MAX_WIDTH, MAX_HEIGHT);

	LogDebug("Setting window icon...\n");
	windowIcon = ToSDLSurface(TEXTURE("interface/icon"));
	SDL_SetWindowIcon(window, windowIcon);

	SetWindowFocused((SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) != 0);
}

void HandleEvent(void)
{
	switch (event.type)
	{
		case SDL_EVENT_QUIT:
			shouldQuit = true;
			break;
		case SDL_EVENT_KEY_UP:
			HandleKeyUp(event.key.scancode);
			break;
		case SDL_EVENT_KEY_DOWN:
			HandleKeyDown(event.key.scancode);
			break;
		case SDL_EVENT_MOUSE_MOTION:
			HandleMouseMotion((int)event.motion.x, (int)event.motion.y, (int)event.motion.xrel, (int)event.motion.yrel);
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			HandleMouseUp(event.button.button);
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			HandleMouseDown(event.button.button);
			break;
		case SDL_EVENT_WINDOW_RESIZED:
		case SDL_EVENT_WINDOW_MAXIMIZED:
			UpdateViewportSize();
			break;
		case SDL_EVENT_WINDOW_RESTORED:
			WindowRestored();
			break;
		case SDL_EVENT_WINDOW_MINIMIZED:
			WindowObscured();
			break;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			SetWindowFocused(false);
			break;
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			SetWindowFocused(true);
			break;
		case SDL_EVENT_GAMEPAD_ADDED:
			HandleGamepadConnect();
			break;
		case SDL_EVENT_GAMEPAD_REMOVED:
			HandleGamepadDisconnect(event.cdevice.which);
			break;
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			HandleControllerButtonDown(event.gbutton.button);
			break;
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			HandleControllerButtonUp(event.gbutton.button);
			break;
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			HandleControllerAxis(event.gaxis.axis, event.gaxis.value);
			break;
		case SDL_EVENT_TEXT_INPUT:
			HandleTextInput(&event.text);
			break;
		default:
			break;
	}
}

void InitEngine(const int argc, const char *argv[], const RegisterGameActorsFunction RegisterGameActors)
{
	ErrorHandlerInit();
	ExecPathInit(argc, argv);
	LogInit();
	LogInfo("Build time: %s at %s\n", __DATE__, __TIME__);
	LogInfo("Engine Version: %s\n", ENGINE_VERSION);
	LogInfo("Initializing Engine\n");

	InitArguments(argc, argv);

	LoadGameConfig();

	InitOptions();

	InitTimers();

	if (HasCliArg("--renderer"))
	{
		const char *renderer = GetCliArgStr("--renderer", "gl");
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

	RegisterActors(RegisterGameActors);

	InitState();
	PhysicsThreadInit();

	if (!RenderPreInit())
	{
		RenderInitError();
	}

	InitSoundSystem();

	WindowAndRenderInit();

	InitCommonFonts();

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
			SDL_Delay(LOW_FPS_MODE_SLEEP_MS);
		}
		return;
	}

	ClearScreen();

	ResetDPrintYPos();

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
		SDL_Delay(LOW_FPS_MODE_SLEEP_MS);
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
	SDL_DestroySurface(windowIcon);
	DestroyCommonFonts();
	DestroyAssetCache(); // Free all assets
	LogDebug("Cleaning up SDL...\n");
	SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);
	SDL_Quit();
	LogDestroy();
}

bool EngineShouldQuit()
{
	return shouldQuit;
}
