//
// Created by droc101 on 4/22/2024.
//

#include "GlobalState.h"
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../GameStates/GLevelSelectState.h"
#include "../GameStates/GMenuState.h"
#include "../GameStates/GOptionsState.h"
#include "../GameStates/GPauseState.h"
#include "../GameStates/Options/GInputOptionsState.h"
#include "../GameStates/Options/GSoundOptionsState.h"
#include "../GameStates/Options/GVideoOptionsState.h"
#include "../Helpers/Core/AssetLoaders/LevelLoader.h"
#include "../Helpers/Core/AssetLoaders/ModelLoader.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Core/Physics/Physics.h"
#include "../Helpers/Core/Physics/PhysicsThread.h"
#include "../Helpers/Core/Physics/Player.h"
#include "../Helpers/Core/SoundSystem.h"
#include "../Helpers/Discord.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/Level.h"
#include "Asset.h"
#include "Camera.h"
#include "Options.h"

#define MAX_HEALTH 100

static GlobalState state;

void InitOptions()
{
	LogDebug("Loading options...\n");
	LoadOptions(&state.options);
}

void InitState()
{
	LogDebug("Initializing global state...\n");
	state.saveData = calloc(1, sizeof(SaveData));
	CheckAlloc(state.saveData);
	state.saveData->hp = 100;
	state.level = CreateLevel(); // empty level so we don't segfault
	state.levelName = calloc(1, 1);
	state.camera = calloc(1, sizeof(Camera));
	CheckAlloc(state.camera);
	state.camera->fov = GetState()->options.fov;
	state.rpcState = IN_MENUS;

	state.viewmodel.enabled = true;
	state.viewmodel.model = LoadModel(MODEL("eraser"));
	state.viewmodel.transform.position.x = 0.5f;
	JPH_Quat_Rotation(&Vector3_AxisY, degToRad(5), &state.viewmodel.transform.rotation);
}

inline GlobalState *GetState()
{
	return &state;
}

void TakeDamage(const int damage)
{
	state.saveData->hp -= damage;
	if (state.saveData->hp < 0)
	{
		state.saveData->hp = 0;
	}
}

void Heal(const int amount)
{
	state.saveData->hp += amount;
	if (state.saveData->hp > MAX_HEALTH)
	{
		state.saveData->hp = MAX_HEALTH;
	}
}

void SetStateCallbacks(const FrameUpdateFunction UpdateGame,
					   const FixedUpdateFunction FixedUpdateGame,
					   const CurrentState currentState,
					   const FrameRenderFunction RenderGame)
{
	state.physicsFrame = 0;
	state.UpdateGame = UpdateGame;
	state.currentState = currentState;
	state.RenderGame = RenderGame;
	PhysicsThreadSetFunction(FixedUpdateGame);
	DiscordUpdateRPC();
}

void ChangeLevel(Level *level, char *levelName)
{
	if (!level)
	{
		LogError("Cannot change to a NULL level. Something might have gone wrong while loading it.\n");
		return;
	}
	PhysicsThreadLockTickMutex();
	if (state.level)
	{
		DestroyLevel(state.level);
		free(state.levelName);
	}
	state.level = level;
	state.levelName = levelName;
	if (strncmp(level->music, "none", 4) != 0)
	{
		char musicPath[80];
		snprintf(musicPath, sizeof(musicPath), SOUND("%s"), level->music);
		ChangeMusic(musicPath);
	} else
	{
		StopMusic();
	}

	LoadLevelWalls(level);
	PhysicsThreadUnlockTickMutex();
}

void DestroyGlobalState()
{
	LogDebug("Cleaning up GlobalState...\n");
	SaveOptions(&state.options);
	DestroyLevel(state.level);
	free(state.saveData);
	free(state.camera);

	LogDebug("Cleaning up game states...\n");
	GInputOptionsStateDestroy();
	GSoundOptionsStateDestroy();
	GVideoOptionsStateDestroy();
	GLevelSelectStateDestroy();
	GMenuStateDestroy();
	GOptionsStateDestroy();
	GPauseStateDestroy();
	PhysicsDestroyGlobal(&state);
}

bool ChangeLevelByName(const char *name)
{
	LogInfo("Loading level \"%s\"\n", name);

	const int maxPathLength = 80;
	char *levelPath = calloc(maxPathLength, sizeof(char));
	CheckAlloc(levelPath);

	if (snprintf(levelPath, maxPathLength, LEVEL("%s"), name) > maxPathLength)
	{
		LogError("Failed to load level due to level name %s being too long\n", name);
		free(levelPath);
		return false;
	}
	Asset *levelData = DecompressAsset(levelPath, false);
	free(levelPath);
	if (levelData == NULL)
	{
		LogError("Failed to load level asset.\n");
		return false;
	}
	GetState()->saveData->blueCoins = 0;
	ChangeLevel(LoadLevel(levelData->data, levelData->size), strdup(name));
	FreeAsset(levelData);
	DiscordUpdateRPC();
	return true;
}
