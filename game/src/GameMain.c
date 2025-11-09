#include <engine/Engine.h>
#include <engine/helpers/Arguments.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stdint.h>
#include "actor/Laser.h"
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

void DestroyGame()
{
	InputOptionsStateDestroy();
	SoundOptionsStateDestroy();
	VideoOptionsStateDestroy();
	LevelSelectStateDestroy();
	MenuStateDestroy();
	OptionsStateDestroy();
	PauseStateDestroy();
	LaserRaycastFiltersDestroy();
}

EXPORT_SYM int GameMain(const int argc, const char *argv[])
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
	DestroyGame();
	DestroyEngine();
	return 0;
}
