//
// Created by droc101 on 5/29/25.
//

#include "SoundPlayer.h"
#include <joltc/Math/Transform.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/Error.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Helpers/Core/SoundSystem.h"
#include "../../Structs/Actor.h"
#include "../../Structs/ActorDefinition.h"

enum SoundPlayerInput
{
	SOUND_PLAYER_INPUT_PLAY = 1,
	SOUND_PLAYER_INPUT_PAUSE = 2,
	SOUND_PLAYER_INPUT_RESUME = 3,
	SOUND_PLAYER_INPUT_STOP = 4,
};

typedef struct SoundPlayerData
{
	char asset[80]; // asset name of the sound effect to play
	SoundEffect *effect;
	int loops;
	float volume;
} SoundPlayerData;

static void SoundPlayerSoundDone(void *pData)
{
	if (pData)
	{
		((SoundPlayerData *)pData)->effect = NULL;
	}
}

static void SoundPlayerDestroy(Actor *this)
{
	const SoundPlayerData *data = this->extraData;
	if (data->effect)
	{
		StopSoundEffect(data->effect);
	}
}

static bool SoundPlayerSignalHandler(Actor *this, const Actor *sender, const uint8_t signal, const Param *param)
{
	if (DefaultActorSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	SoundPlayerData *data = this->extraData;
	if (signal == SOUND_PLAYER_INPUT_PLAY)
	{
		data->effect = PlaySoundEffect(data->asset, data->loops, data->volume, SoundPlayerSoundDone, data);
		return true;
	}
	if (data->effect)
	{
		if (signal == SOUND_PLAYER_INPUT_PAUSE)
		{
			PauseSoundEffect(data->effect);
			return true;
		}
		if (signal == SOUND_PLAYER_INPUT_RESUME)
		{
			ResumeSoundEffect(data->effect);
			return true;
		}
		if (signal == SOUND_PLAYER_INPUT_STOP)
		{
			StopSoundEffect(data->effect);
			return true;
		}
	}
	return false;
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_SOUND_PLAYER,
	.Update = DefaultActorUpdate,
	.SignalHandler = SoundPlayerSignalHandler,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = SoundPlayerDestroy,
};

void SoundPlayerInit(Actor *this, const KvList *params, Transform * /*transform*/)
{
	this->definition = &definition;

	SoundPlayerData *data = calloc(1, sizeof(SoundPlayerData));
	CheckAlloc(data);
	data->effect = NULL;
	snprintf(data->asset, sizeof(data->asset), SOUND("%s"), KvGetString(params, "sound", "sfx/click"));
	data->loops = KvGetInt(params, "loops", 0);
	data->volume = KvGetFloat(params, "volume", 1);
	this->extraData = data;
}
