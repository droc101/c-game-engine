//
// Created by droc101 on 8/4/25.
//

#include <engine/assets/AssetReader.h>
#include <engine/structs/Asset.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/SoundSystem.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_properties.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// TODO it doesnt work. completely silent.

struct SoundChannel
{
	MIX_Audio *chunk;
	MIX_Track *track;
	uint8_t channelIndex;
	SoundFinishedCallback callback;
	void *callbackData;
	float originalVolume;
};

struct SoundSystem
{
	/// mixer instance
	MIX_Mixer *mixer;
	/// Whether the audio system has been started successfully
	bool isAudioStarted;
	/// sound tracks/channels
	MIX_Track *tracks[SFX_CHANNEL_COUNT];
	/// currently playing sounds
	SoundChannel *sounds[SFX_CHANNEL_COUNT];
	/// The user's chosen sound effect volume
	float sfxVolume;
};

SoundSystem soundSys;

/**
 * callback for when a channel finishes playing (so we can free it)
 */
void ChannelFinished(void *userdata, MIX_Track *track)
{
	if (!userdata)
	{
		LogWarning("ChannelFinished called with NULL userdata!\n");
		return;
	}
	SoundChannel *effect = userdata;
	if (effect->callback)
	{
		effect->callback(effect->callbackData);
	}
	MIX_DestroyAudio(effect->chunk);
	soundSys.sounds[effect->channelIndex] = NULL;
	free(effect);
}

void InitSoundSystem()
{
	LogDebug("Initializing sound system...\n");
	if (!MIX_Init())
	{
		LogError("MIX_Init failed with error %s\n", SDL_GetError());
		soundSys.isAudioStarted = false;
		return;
	}
	int numAudioDevices = 0;
	SDL_GetAudioPlaybackDevices(&numAudioDevices);
	if (numAudioDevices == 0)
	{
		soundSys.isAudioStarted = false;
		LogWarning("Not starting sound system because there are no output devices.\n");
		return;
	}
	const SDL_AudioSpec spec = {.channels = 2, .freq = 48000, .format = SDL_AUDIO_S16LE};
	soundSys.mixer = MIX_CreateMixer(&spec);
	if (!soundSys.mixer)
	{
		soundSys.isAudioStarted = false;
		LogError("Mix_OpenAudio Error: %s\n", SDL_GetError());
	} else
	{
		soundSys.isAudioStarted = true;
	}

	for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
	{
		soundSys.sounds[i] = NULL;
		soundSys.tracks[i] = MIX_CreateTrack(soundSys.mixer);
		if (!soundSys.tracks[i])
		{
			LogError("MIX_CreateTrack Error: %s\n", SDL_GetError());
			soundSys.isAudioStarted = false;
			return;
		}
		MIX_SetTrackStoppedCallback(soundSys.tracks[i], NULL, NULL);
	}
	UpdateVolume();
}

void DestroySoundSystem()
{
	LogDebug("Cleaning up sound system...\n");

	// free sound effects
	if (soundSys.isAudioStarted)
	{
		for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
		{
			SoundChannel *channel = soundSys.sounds[i];
			if (channel)
			{
				MIX_SetTrackStoppedCallback(channel->track, NULL, NULL);
				MIX_StopTrack(channel->track, 0);
				MIX_DestroyAudio(channel->chunk);
				free(channel);
			}
		}
	}

	MIX_DestroyMixer(soundSys.mixer);
	MIX_Quit();
}

void UpdateVolume()
{
	const float sfxVol = GetState()->options.sfxVolume * GetState()->options.masterVolume;
	const float musicVol = GetState()->options.musicVolume * GetState()->options.masterVolume;
	soundSys.sfxVolume = sfxVol;
	for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
	{
		const SoundChannel *effect = soundSys.sounds[i];
		if (effect)
		{
			const float mixedVolume = effect->originalVolume * sfxVol;
			MIX_SetTrackGain(effect->track, mixedVolume);
		}
	}
}

MIX_Track *FindAvailableTrack(uint8_t *index)
{
	for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
	{
		if (!MIX_TrackPlaying(soundSys.tracks[i]))
		{
			if (soundSys.sounds[i] != NULL)
			{
				LogWarning("SoundSystem: MIX_TrackPlaying desynced with soundSys.effects NULL slots!\n");
				continue;
			}
			if (index)
			{
				*index = i;
			}
			return soundSys.tracks[i];
		}
	}
	return NULL;
}

SoundChannel *PlaySound(const char *asset,
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
	SDL_IOStream *stream = SDL_IOFromMem(wav->data, wav->size);
	if (!stream)
	{
		LogError("SDL_IOFromConstMem Error: %s\n", SDL_GetError());
		return NULL;
	}
	MIX_Audio *audio = MIX_LoadAudio_IO(soundSys.mixer,
										stream,
										true,
										true); // TODO predecode should be an arg to PlaySoundEffect
	if (audio == NULL)
	{
		LogError("MIX_LoadAudio_IO Error: %s\n", SDL_GetError());
		return NULL;
	}
	uint8_t index = 0;
	MIX_Track *track = FindAvailableTrack(&index);
	if (!track)
	{
		LogError("PlaySoundEffect Error: No available channels.\n");
		MIX_DestroyAudio(audio);
		return NULL;
	}

	SoundChannel *effect = malloc(sizeof(SoundChannel));
	CheckAlloc(effect);
	soundSys.sounds[index] = effect;
	effect->chunk = audio;
	effect->track = track;
	effect->channelIndex = index;
	effect->callback = callback;
	effect->callbackData = callbackData;
	effect->originalVolume = volume;
	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
	MIX_SetTrackAudio(track, audio);
	MIX_SetTrackGain(track,
					 volume * soundSys.sfxVolume); // TODO distinction between sound categories (music vs sfx right now)
	MIX_SetTrackStoppedCallback(track, ChannelFinished, effect);
	if (!MIX_PlayTrack(track, props))
	{
		LogError("MIX_PlayTrack failed: %s\n", SDL_GetError());
	}
	SDL_DestroyProperties(props);

	return effect;
}

inline void PauseSound(const SoundChannel *effect)
{
	MIX_PauseTrack(effect->track);
}

inline void ResumeSound(const SoundChannel *effect)
{
	MIX_ResumeTrack(effect->track);
}

inline void StopSound(const SoundChannel *effect)
{
	if (!MIX_StopTrack(effect->track, 0)) // TODO argument for fade_out_frames?
	{
		LogError("Mix_HaltChannel failed: %s", SDL_GetError());
	}
}
