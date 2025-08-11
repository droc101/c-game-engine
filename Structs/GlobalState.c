//
// Created by droc101 on 4/22/2024.
//

#include "GlobalState.h"
#include <stdio.h>
#include <string.h>
#include "../Debug/JoltDebugRenderer.h"
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
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/Level.h"
#include "Options.h"

static GlobalState state;

void InitOptions()
{
	LoadOptions(&state.options);
}

void InitState()
{
	PhysicsInitGlobal(&state);
	state.saveData = calloc(1, sizeof(SaveData));
	CheckAlloc(state.saveData);
	state.saveData->hp = 100;
	state.level = CreateLevel(); // empty level so we don't segfault
	state.camera = calloc(1, sizeof(Camera));
	CheckAlloc(state.camera);
	state.camera->fov = FOV;

	state.viewmodel.enabled = true;
	state.viewmodel.model = LoadModel(MODEL("eraser"));
	state.viewmodel.transform.position.x = 0.5f;
	state.viewmodel.transform.rotation.y = degToRad(5);
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
}

void ChangeLevel(Level *level)
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
	}
	state.level = level;
	if (strncmp(level->music, "none", 4) != 0)
	{
		char musicPath[92];
		snprintf(musicPath, 92, "audio/%s.gmus", level->music);
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
	SaveOptions(&state.options);
	DestroyLevel(state.level);
	free(state.saveData);
	free(state.camera);

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

	if (snprintf(levelPath, maxPathLength, "level/%s.gmap", name) > maxPathLength)
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
	ChangeLevel(LoadLevel(levelData->data, levelData->size));
	FreeAsset(levelData);
	return true;
}
