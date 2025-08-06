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
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Core/Physics/PhysicsInit.h"
#include "../Helpers/Core/Physics/PhysicsThread.h"
#include "../Helpers/Core/Physics/Player.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Helpers/LevelLoader.h"
#include "../Structs/Level.h"
#include "Options.h"

static GlobalState state;

/**
 * callback for when a channel finishes playing (so we can free it)
 * @param channel The channel that finished
 */
void ChannelFinished(const int channel)
{
	if (state.channels[channel] != NULL)
	{
		Mix_FreeChunk(state.channels[channel]);
		state.channels[channel] = NULL;
	} else
	{
		LogWarning("A sound effect channel finished, but it was already NULL!\n");
	}
}

void InitOptions()
{
	LoadOptions(&state.options);
}

void InitState()
{
	InitJolt();
	state.saveData = calloc(1, sizeof(SaveData));
	CheckAlloc(state.saveData);
	state.saveData->hp = 100;
	state.level = CreateLevel(); // empty level so we don't segfault
	state.camera = calloc(1, sizeof(Camera));
	CheckAlloc(state.camera);
	state.camera->fov = FOV;

	state.viewmodel.enabled = true;
	state.viewmodel.model = LoadModel(MODEL("model_eraser"));
	state.viewmodel.transform.position.x = 0.5f;
	state.viewmodel.transform.rotation.y = degToRad(5);

	UpdateVolume();

	StopMusic();
	Mix_ChannelFinished(ChannelFinished);
}

void UpdateVolume()
{
	const double sfxVol = state.options.sfxVolume * state.options.masterVolume;
	const double musicVol = state.options.musicVolume * state.options.masterVolume;
	Mix_Volume(-1, (int)(sfxVol * MIX_MAX_VOLUME));
	Mix_VolumeMusic((int)(musicVol * MIX_MAX_VOLUME));
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

void ChangeMusic(const char *asset)
{
	if (!state.isAudioStarted)
	{
		return;
	}

	StopMusic(); // stop the current music and free its data
	const Asset *mp3 = DecompressAsset(asset, true);
	if (mp3 == NULL)
	{
		LogError("Failed to load music asset.\n");
		return;
	}

	if (mp3->type != ASSET_TYPE_MP3)
	{
		LogWarning("ChangeMusic Error: Asset is not a music file.\n");
		return;
	}

	const uint mp3Size = mp3->size;
	Mix_Music *mus = Mix_LoadMUS_RW(SDL_RWFromConstMem(mp3->data, (int)mp3Size), 1);
	if (mus == NULL)
	{
		printf("Mix_LoadMUS_RW Error: %s\n", Mix_GetError());
		return;
	}
	state.music = mus;
	Mix_FadeInMusic(mus, -1, 500);
}

void StopMusic()
{
	if (!state.isAudioStarted)
	{
		return;
	}
	if (state.music != NULL)
	{
		// stop and free the current music
		Mix_HaltMusic();
		Mix_FreeMusic(state.music);
		state.music = NULL; // set to NULL, so we don't free it again if this function fails
	}
}

void PlaySoundEffect(const char *asset)
{
	if (!state.isAudioStarted)
	{
		return;
	}

	const Asset *wav = DecompressAsset(asset, true);
	if (wav == NULL)
	{
		LogError("Failed to load sound effect asset.\n");
		return;
	}
	if (wav->type != ASSET_TYPE_WAV)
	{
		LogError("PlaySoundEffect Error: Asset is not a sound effect file.\n");
		return;
	}
	const uint wavSize = wav->size;
	Mix_Chunk *chunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(wav->data, (int)wavSize), 1);
	if (chunk == NULL)
	{
		LogError("Mix_LoadWAV_RW Error: %s\n", Mix_GetError());
		return;
	}
	for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
	{
		if (state.channels[i] == NULL)
		{
			state.channels[i] = chunk;
			Mix_PlayChannel(i, chunk, 0);
			return;
		}
	}
	LogError("PlaySoundEffect Error: No available channels.\n");
	Mix_FreeChunk(chunk);
}

void DestroyGlobalState()
{
	SaveOptions(&state.options);
	DestroyLevel(state.level);
	free(state.saveData);
	free(state.camera);
	if (state.music != NULL)
	{
		Mix_HaltMusic();
		Mix_FreeMusic(state.music);
	}

	Mix_ChannelFinished(NULL);
	Mix_HaltChannel(-1);

	// free sound effects
	if (state.isAudioStarted)
	{
		for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
		{
			if (state.channels[i] != NULL)
			{
				Mix_FreeChunk(state.channels[i]);
			}
		}
	}

	GInputOptionsStateDestroy();
	GSoundOptionsStateDestroy();
	GVideoOptionsStateDestroy();
	GLevelSelectStateDestroy();
	GMenuStateDestroy();
	GOptionsStateDestroy();
	GPauseStateDestroy();

	JoltDebugRendererDestroy();
	PlayerContactListenerDestroy();
	JPH_JobSystem_Destroy(state.jobSystem);
	JPH_Shutdown();
}

bool ChangeLevelByName(const char *name)
{
	LogInfo("Loading level \"%s\"\n", name);

	const size_t maxPathLength = 80;
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
