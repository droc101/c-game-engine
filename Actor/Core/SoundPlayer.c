//
// Created by droc101 on 5/29/25.
//

#include "SoundPlayer.h"
#include <joltc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../Helpers/Core/Error.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Helpers/Core/SoundSystem.h"
#include "../../Structs/Actor.h"

#define SOUNDPLAYER_INPUT_PLAY 1
#define SOUNDPLAYER_INPUT_PAUSE 2
#define SOUNDPLAYER_INPUT_RESUME 3
#define SOUNDPLAYER_INPUT_STOP 4


typedef struct SoundPlayerData
{
	char asset[64]; // asset name of the sound effect to play
	SoundEffect *effect;
	int loops;
	float volume;
} SoundPlayerData;

void SoundPlayerSoundDone(void *pData)
{
	if (pData)
	{
		((SoundPlayerData *)pData)->effect = NULL;
	}
}

bool SoundPlayerSignalHandler(Actor *this, const Actor *sender, const uint8_t signal, const Param *param)
{
	if (DefaultSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	SoundPlayerData *data = this->extraData;
	if (signal == SOUNDPLAYER_INPUT_PLAY)
	{
		data->effect = PlaySoundEffect(data->asset, data->loops, data->volume, SoundPlayerSoundDone, data);
		return true;
	}
	if (data->effect)
	{
		if (signal == SOUNDPLAYER_INPUT_PAUSE)
		{
			PauseSoundEffect(data->effect);
			return true;
		}
		if (signal == SOUNDPLAYER_INPUT_RESUME)
		{
			ResumeSoundEffect(data->effect);
			return true;
		}
		if (signal == SOUNDPLAYER_INPUT_STOP)
		{
			StopSoundEffect(data->effect);
			return true;
		}
	}
	return false;
}

void SoundPlayerInit(Actor *this, const KvList *params, Transform *)
{
	this->SignalHandler = SoundPlayerSignalHandler;
	SoundPlayerData *data = calloc(1, sizeof(SoundPlayerData));
	CheckAlloc(data);
	data->effect = NULL;
	snprintf(data->asset, sizeof(data->asset), "audio/%s.gsnd", KvGetString(params, "sound", "sfx_click"));
	data->loops = KvGetInt(params, "loops", 0);
	data->volume = KvGetFloat(params, "volume", 1);
	this->extraData = data;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void SoundPlayerDestroy(Actor *this)
{
	const SoundPlayerData *data = this->extraData;
	if (data->effect)
	{
		StopSoundEffect(data->effect);
	}
}
