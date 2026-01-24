//
// Created by droc101 on 8/4/25.
//

#include <engine/assets/AssetReader.h>
#include <engine/structs/Asset.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/subsystem/threads/PhysicsThread.h>
#include <joltc/Math/Vector3.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

struct SoundChannel
{
	/// The audio playing on this channel
	MIX_Audio *audio;
	/// The track this audio is playing on
	MIX_Track *track;
	/// The index of the channel this audio is playing on
	uint8_t channelIndex;

	/// The function to call when this channel finishes
	SoundFinishedCallback callback;
	/// The data to pass to @c callback
	void *callbackData;

	/// The original volume of this channel, unmodified by user volume settings
	float originalVolume;
	/// The sound category of this channel
	SoundCategory category;

	/// The original position in 3D space, as specified in the sound request
	Vector3 originalPosition;
	/// The SDL point associated with the track, or NULL if this channel is not positioned
	MIX_Point3D *position;
};

struct SoundSystem
{
	/// mixer instance
	MIX_Mixer *mixer;
	/// Whether the audio system has been started successfully
	bool isAudioStarted;
	/// sound tracks/channels
	MIX_Track *tracks[SOUND_SYSTEM_CHANNEL_COUNT];
	/// currently playing sounds
	SoundChannel *channels[SOUND_SYSTEM_CHANNEL_COUNT];
};

SoundSystem soundSys;

/**
 * callback for when a channel finishes playing (so we can free it)
 */
void ChannelFinished(void *userdata, MIX_Track * /*track*/)
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
	if (effect->position)
	{
		free(effect->position);
	}
	MIX_DestroyAudio(effect->audio);
	soundSys.channels[effect->channelIndex] = NULL;
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
	SDL_AudioDeviceID *devices = SDL_GetAudioPlaybackDevices(&numAudioDevices);
	SDL_free(devices); // SDL, why did you remove the function that just got the count with no funny stuff
	if (numAudioDevices == 0)
	{
		soundSys.isAudioStarted = false;
		LogWarning("Not starting sound system because there are no output devices.\n");
		return;
	}

	soundSys.mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
	if (!soundSys.mixer)
	{
		soundSys.isAudioStarted = false;
		LogError("MIX_CreateMixerDevice Error: %s\n", SDL_GetError());
	} else
	{
		soundSys.isAudioStarted = true;
	}

	for (int i = 0; i < SOUND_SYSTEM_CHANNEL_COUNT; i++)
	{
		soundSys.channels[i] = NULL;
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

	if (soundSys.isAudioStarted)
	{
		for (int i = 0; i < SOUND_SYSTEM_CHANNEL_COUNT; i++)
		{
			SoundChannel *channel = soundSys.channels[i];
			if (channel)
			{
				MIX_SetTrackStoppedCallback(channel->track, NULL, NULL);
				MIX_StopTrack(channel->track, 0);
				MIX_DestroyAudio(channel->audio);
				free(channel);
			}
		}
	}

	MIX_DestroyMixer(soundSys.mixer);
	MIX_Quit();
}

void UpdateSoundSystem()
{
	Vector3 listener = Vector3_Zero;
	if (GetState()->map)
	{
		PhysicsThreadLockTickMutex();
		listener = GetState()->map->player.transform.position;
		PhysicsThreadUnlockTickMutex();
	}
	for (int i = 0; i < SOUND_SYSTEM_CHANNEL_COUNT; i++)
	{
		if (MIX_TrackPlaying(soundSys.tracks[i]))
		{
			if (soundSys.channels[i] == NULL)
			{
				LogWarning("SoundSystem: MIX_TrackPlaying desynced with soundSys.effects NULL slots!\n");
				continue;
			}
			const SoundChannel *channel = soundSys.channels[i];
			if (channel->position)
			{
				Vector3 relPosition;
				// TODO calculation seems wrong
				Vector3_Subtract(&listener, &channel->originalPosition, &relPosition);
				channel->position->x = relPosition.x;
				channel->position->y = relPosition.y;
				channel->position->z = relPosition.z;
				MIX_SetTrack3DPosition(channel->track, channel->position);
			}
		}
	}
}

float GetCategoryVolume(const SoundCategory category)
{
	float categoryVolumeMultiplier = 1.0f;
	switch (category)
	{
		case SOUND_CATEGORY_MUSIC:
			categoryVolumeMultiplier = GetState()->options.musicVolume;
			break;
		case SOUND_CATEGORY_SFX:
			categoryVolumeMultiplier = GetState()->options.sfxVolume;
			break;
		case SOUND_CATEGORY_UI:
			categoryVolumeMultiplier = GetState()->options.uiVolume;
			break;
	}

	return categoryVolumeMultiplier * GetState()->options.masterVolume;
}

void UpdateVolume()
{
	for (int i = 0; i < SOUND_SYSTEM_CHANNEL_COUNT; i++)
	{
		const SoundChannel *effect = soundSys.channels[i];
		if (effect)
		{
			const float mixedVolume = effect->originalVolume * GetCategoryVolume(effect->category);
			MIX_SetTrackGain(effect->track, mixedVolume);
		}
	}
}

MIX_Track *FindAvailableTrack(uint8_t *index)
{
	for (int i = 0; i < SOUND_SYSTEM_CHANNEL_COUNT; i++)
	{
		if (!MIX_TrackPlaying(soundSys.tracks[i]))
		{
			if (soundSys.channels[i] != NULL)
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

SoundChannel *PlaySound(const char *soundAsset, const SoundCategory category)
{
	const SoundRequest request = {
		.soundAsset = soundAsset,
		.category = category,
		.volume = 1.0f,
		.completionCallback = NULL,
		.completionCallbackData = NULL,
		.numLoops = 0,
		.preload = false,
	};
	return PlaySoundEx(&request);
}

SoundChannel *PlaySoundEx(const SoundRequest *request)
{
	if (!soundSys.isAudioStarted)
	{
		return NULL;
	}
	const Asset *wav = DecompressAsset(request->soundAsset, true);
	if (wav == NULL)
	{
		LogError("Failed to load sound effect asset.\n");
		return NULL;
	}
	if (wav->type != ASSET_TYPE_WAV)
	{
		LogError("PlaySoundEx Error: Asset is not a sound effect file.\n");
		return NULL;
	}
	SDL_IOStream *stream = SDL_IOFromConstMem(wav->data, wav->size);
	if (!stream)
	{
		LogError("SDL_IOFromConstMem Error: %s\n", SDL_GetError());
		return NULL;
	}
	MIX_Audio *audio = MIX_LoadAudio_IO(soundSys.mixer, stream, request->preload, true);
	if (audio == NULL)
	{
		LogError("MIX_LoadAudio_IO Error: %s\n", SDL_GetError());
		return NULL;
	}
	uint8_t index = 0;
	MIX_Track *track = FindAvailableTrack(&index);
	if (!track)
	{
		LogError("PlaySoundEffect Error: No available tracks.\n");
		MIX_DestroyAudio(audio);
		return NULL;
	}

	SoundChannel *effect = malloc(sizeof(SoundChannel));
	CheckAlloc(effect);
	soundSys.channels[index] = effect;
	effect->audio = audio;
	effect->track = track;
	effect->channelIndex = index;
	effect->category = request->category;
	effect->callback = request->completionCallback;
	effect->callbackData = request->completionCallbackData;
	effect->originalVolume = request->volume;
	if (request->positional)
	{
		MIX_Point3D *point = malloc(sizeof(MIX_Point3D));
		CheckAlloc(point);
		point->x = request->position.x;
		point->y = request->position.y;
		point->z = request->position.z;
		effect->position = point;
		effect->originalPosition = request->position;
	} else
	{
		effect->position = NULL;
	}
	const SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, request->numLoops);
	MIX_SetTrackAudio(track, audio);
	const float volume = request->volume * GetCategoryVolume(request->category);
	MIX_SetTrackGain(track, volume);
	MIX_SetTrack3DPosition(track, effect->position);
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
		LogError("MIX_StopTrack failed: %s", SDL_GetError());
	}
}
