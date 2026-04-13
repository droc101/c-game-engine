#include <engine/Engine.h>
#include <engine/helpers/Arguments.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include "actor/Laser.h"
#include "gameState/LogoSplashState.h"
#include "gameState/MainState.h"
#include "gameState/MenuState.h"
#include "helpers/GameActorRegistration.h"

#undef main // Leaked by SDL_main.h

void SetInitialGameState()
{
	bool loadMap = false;
	if (HasCliArg("--map"))
	{
		const char *mapName = GetCliArgStr("--map", "");
		if (ChangeMapByName(mapName))
		{
			loadMap = true;
		}
	}
	if (loadMap)
	{
		SetGameState(&MainState);
	} else
	{
		if (!HasCliArg("--nosplash"))
		{
			SetGameState(&LogoSplashState);
		} else
		{
			menuStateFadeIn = false;
			SetGameState(&MenuState);
		}
	}
}

void DestroyGame()
{
	LaserRaycastFiltersDestroy();
}

EXPORT_SYM int GameMain(const int argc, const char *argv[])
{
	InitEngine(argc, argv, RegisterGameActors);
	LaserRaycastFiltersInit();
	SetInitialGameState();
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
