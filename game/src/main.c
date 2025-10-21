#include <engine/Engine.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stdint.h>
#include "actor/Laser.h"
#include "engine/helpers/Arguments.h"
#include "engine/structs/GlobalState.h"
#include "gameState/LevelSelectState.h"
#include "gameState/LogoSplashState.h"
#include "gameState/MainState.h"
#include "gameState/MenuState.h"
#include "gameState/options/InputOptionsState.h"
#include "gameState/options/SoundOptionsState.h"
#include "gameState/options/VideoOptionsState.h"
#include "gameState/OptionsState.h"
#include "gameState/PauseState.h"
#include "helpers/GameActorRegistration.h"

#undef main // Leaked by SDL_main.h

void SetInitialGameState(const int argc, const char *argv[])
{
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
		MainStateSet();
	} else
	{
		if (!HasCliArg(argc, argv, "--nosplash"))
		{
			LogoSplashStateSet();
		} else
		{
			MenuStateSet();
		}
	}
}

int main(const int argc, const char *argv[])
{
	InitEngine(argc, argv, RegisterGameActors);
	LaserRaycastFiltersInit();
	SetInitialGameState(argc, argv);
	LogInfo("Engine initialized, entering mainloop\n");
	while (!EngineShouldQuit())
	{
		EngineIteration();
	}
	LogInfo("Mainloop exited, cleaning up engine...\n");
	InputOptionsStateDestroy();
	SoundOptionsStateDestroy();
	VideoOptionsStateDestroy();
	LevelSelectStateDestroy();
	MenuStateDestroy();
	OptionsStateDestroy();
	PauseStateDestroy();
	LaserRaycastFiltersDestroy();
	DestroyEngine();
	return 0;
}

#ifdef WIN32
/// Make this symbol exported (in the symbol table)
#define EXPORT_SYM __declspec(dllexport)
#else
/// Make this symbol exported (in the symbol table)
#define EXPORT_SYM __attribute__((visibility("default")))
#endif

// Exporting these symbols tells GPU drivers to use the dedicated GPU on hybrid systems
// I do not know if these do anything on Linux, but they are here just in case.
EXPORT_SYM uint32_t NvOptimusEnablement = 0x00000001;
EXPORT_SYM int AmdPowerXpressRequestHighPerformance = 1;
