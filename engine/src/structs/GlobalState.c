//
// Created by droc101 on 4/22/2024.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/MapLoader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Asset.h>
#include <engine/structs/Camera.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Map.h>
#include <engine/structs/Options.h>
#include <engine/structs/Player.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/subsystem/threads/PhysicsThread.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Vector3.h>
#include <SDL_mouse.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	state.map = CreateMap(); // empty map so we don't segfault
	state.mapName = calloc(1, 1);
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
					   const GameStateId currentState,
					   const FrameRenderFunction RenderGame,
					   const SDL_bool enableRelativeMouseMode)
{
	state.physicsFrame = 0;
	state.UpdateGame = UpdateGame;
	state.currentState = currentState;
	state.RenderGame = RenderGame;
	PhysicsThreadSetFunction(FixedUpdateGame);
	DiscordUpdateRPC();
	SDL_SetRelativeMouseMode(enableRelativeMouseMode);
}

void ChangeMap(Map *map, char *mapName)
{
	if (!map)
	{
		LogError("Cannot change to a NULL map. Something might have gone wrong while loading it.\n");
		return;
	}
	PhysicsThreadLockTickMutex();
	if (state.map)
	{
		DestroyMap(state.map);
		free(state.mapName);
	}
	state.map = map;
	state.mapName = mapName;
	// if (strncmp(level->music, "none", 4) != 0)
	// {
	// 	char musicPath[80];
	// 	snprintf(musicPath, sizeof(musicPath), SOUND("%s"), level->music);
	// 	ChangeMusic(musicPath);
	// } else
	// {
	// 	StopMusic();
	// }

	PhysicsThreadUnlockTickMutex();
}

void DestroyGlobalState()
{
	LogDebug("Cleaning up GlobalState...\n");
	SaveOptions(&state.options);
	DestroyMap(state.map);
	state.map = NULL;
	free(state.saveData);
	free(state.camera);

	LogDebug("Cleaning up game states...\n");
	PhysicsDestroyGlobal(&state);
}

bool ChangeMapByName(const char *name)
{
	LogInfo("Loading map \"%s\"\n", name);

	const int maxPathLength = 80;
	char *mapPath = calloc(maxPathLength, sizeof(char));
	CheckAlloc(mapPath);

	if (snprintf(mapPath, maxPathLength, MAP("%s"), name) > maxPathLength)
	{
		LogError("Failed to load map due to map name %s being too long\n", name);
		free(mapPath);
		return false;
	}
	GetState()->saveData->blueCoins = 0;
	ChangeMap(LoadMap(mapPath), strdup(name));
	free(mapPath);
	DiscordUpdateRPC();
	return true;
}
