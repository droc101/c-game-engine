//
// Created by droc101 on 8/4/25.
//

#include "SoundSystem.h"
#include <SDL_audio.h>
#include <SDL_error.h>
#include <SDL_mixer.h>
#include <SDL_rwops.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../Structs/Asset.h"
#include "../../Structs/GlobalState.h"
#include "AssetReader.h"
#include "Error.h"
#include "Logging.h"

struct SoundEffect
{
	Mix_Chunk *chunk;
	int channel;
	SoundFinishedCallback callback;
	void *callbackData;
	float originalVolume;
};

struct SoundSystem
{
	/// Whether the audio system has been started successfully
	bool isAudioStarted;
	/// background music
	Mix_Music *music;
	/// sound effects
	SoundEffect *channels[SFX_CHANNEL_COUNT];
	/// The user's chosen sound effect volume
	float sfxVolume;
};

SoundSystem soundSys;

/**
 * callback for when a channel finishes playing (so we can free it)
 * @param channel The channel that finished
 */
void ChannelFinished(const int channel)
{
	if (soundSys.channels[channel] != NULL)
	{
		SoundEffect *effect = soundSys.channels[channel];
		if (effect->callback)
		{
			effect->callback(effect->callbackData);
		}
		Mix_FreeChunk(effect->chunk);
		free(effect);
		soundSys.channels[channel] = NULL;
	} else
	{
		LogWarning("A sound effect channel finished, but it was already NULL!\n");
	}
}

void InitSoundSystem()
{
	(void)Mix_AllocateChannels(SFX_CHANNEL_COUNT);

	if (Mix_OpenAudio(48000, AUDIO_S16, 2, 2048) == 0)
	{
		soundSys.isAudioStarted = true;
	} else
	{
		soundSys.isAudioStarted = false;
		LogError("Mix_OpenAudio Error: %s\n", Mix_GetError());
	}

	soundSys.music = NULL;
	for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
	{
		soundSys.channels[i] = NULL;
	}
	UpdateVolume();

	StopMusic();
	Mix_ChannelFinished(ChannelFinished);
}

void DestroySoundSystem()
{
	if (soundSys.music != NULL)
	{
		(void)Mix_HaltMusic();
		Mix_FreeMusic(soundSys.music);
	}

	Mix_ChannelFinished(NULL);
	(void)Mix_HaltChannel(-1);

	// free sound effects
	if (soundSys.isAudioStarted)
	{
		for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
		{
			if (soundSys.channels[i] != NULL)
			{
				Mix_FreeChunk(soundSys.channels[i]->chunk);
			}
		}
	}

	Mix_CloseAudio();
	Mix_Quit();
}

void UpdateVolume()
{
	const float sfxVol = GetState()->options.sfxVolume * GetState()->options.masterVolume;
	const float musicVol = GetState()->options.musicVolume * GetState()->options.masterVolume;
	soundSys.sfxVolume = sfxVol;
	for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
	{
		const SoundEffect *effect = soundSys.channels[i];
		if (effect)
		{
			const float mixedVolume = effect->originalVolume * sfxVol;
			(void)Mix_VolumeChunk(effect->chunk, (int)(mixedVolume * MIX_MAX_VOLUME));
		}
	}
	(void)Mix_VolumeMusic((int)(musicVol * MIX_MAX_VOLUME));
}

void ChangeMusic(const char *asset)
{
	if (!soundSys.isAudioStarted)
	{
		return;
	}

	StopMusic(); // stop the current music and free its data
	const Asset *music = DecompressAsset(asset, true);
	if (music == NULL)
	{
		LogError("Failed to load music asset.\n");
		return;
	}

	if (music->type != ASSET_TYPE_WAV)
	{
		LogWarning("ChangeMusic Error: Asset is not a music file.\n");
		return;
	}

	const size_t musicDataSize = music->size;
	Mix_Music *mus = Mix_LoadMUS_RW(SDL_RWFromConstMem(music->data, (int)musicDataSize), 1);
	if (mus == NULL)
	{
		LogError("Mix_LoadMUS_RW Error: %s\n", Mix_GetError());
		return;
	}
	soundSys.music = mus;
	if (Mix_PlayMusic(mus, -1) != 0)
	{
		LogError("Mix_PlayMusic failed: %s", SDL_GetError());
	}
}

void StopMusic()
{
	if (!soundSys.isAudioStarted)
	{
		return;
	}
	if (soundSys.music != NULL)
	{
		// stop and free the current music
		(void)Mix_HaltMusic();
		Mix_FreeMusic(soundSys.music);
		soundSys.music = NULL; // set to NULL, so we don't free it again if this function fails
	}
}

SoundEffect *PlaySoundEffect(const char *asset,
							 const int loops,
							 const float volume,
							 const SoundFinishedCallback callback,
							 void *callbackData)
{
	if (!soundSys.isAudioStarted)
	{
		return NULL;
	}
	const Asset *wav = DecompressAsset(asset, true);
	if (wav == NULL)
	{
		LogError("Failed to load sound effect asset.\n");
		return NULL;
	}
	if (wav->type != ASSET_TYPE_WAV)
	{
		LogError("PlaySoundEffect Error: Asset is not a sound effect file.\n");
		return NULL;
	}
	const uint32_t wavSize = wav->size;
	Mix_Chunk *chunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(wav->data, (int)wavSize), 1);
	if (chunk == NULL)
	{
		LogError("Mix_LoadWAV_RW Error: %s\n", Mix_GetError());
		return NULL;
	}
	(void)Mix_VolumeChunk(chunk, (int)((volume * soundSys.sfxVolume) * MIX_MAX_VOLUME));
	for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
	{
		if (soundSys.channels[i] == NULL)
		{
			SoundEffect *effect = malloc(sizeof(SoundEffect));
			CheckAlloc(effect);
			effect->chunk = chunk;
			effect->channel = i;
			effect->callback = callback;
			effect->callbackData = callbackData;
			effect->originalVolume = volume;
			soundSys.channels[i] = effect;
			if (Mix_PlayChannel(i, chunk, loops) == -1)
			{
				LogError("Mix_PlayChannel failed: %s", SDL_GetError());
			}
			return effect;
		}
	}
	LogError("PlaySoundEffect Error: No available channels.\n");
	Mix_FreeChunk(chunk);
	return NULL;
}

inline void PauseSoundEffect(const SoundEffect *effect)
{
	Mix_Pause(effect->channel);
}

inline void ResumeSoundEffect(const SoundEffect *effect)
{
	Mix_Resume(effect->channel);
}

inline void StopSoundEffect(const SoundEffect *effect)
{
	if (Mix_HaltChannel(effect->channel) != 0)
	{
		LogError("Mix_HaltChannel failed: %s", SDL_GetError());
	}
}
