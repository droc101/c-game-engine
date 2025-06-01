//
// Created by droc101 on 5/29/25.
//

#include "SoundPlayer.h"
#include "../../Helpers/Core/Error.h"
#include "../../Helpers/Core/KVList.h"
#include "../../Structs/Actor.h"
#include "../../Structs/GlobalState.h"

#define SOUNDPLAYER_INPUT_PLAY 1

typedef struct SoundPlayerData
{
	const char asset[64]; // asset name of the sound effect to play
} SoundPlayerData;

bool SoundPlayerSignalHandler(Actor *this, const Actor *sender, const byte signal, const Param *param)
{
	if (DefaultSignalHandler(this, sender, signal, param))
	{
		return true;
	}
	if (signal == SOUNDPLAYER_INPUT_PLAY)
	{
		const SoundPlayerData *data = this->extraData;
		PlaySoundEffect(data->asset);
		return true;
	}
	return false;
}

void SoundPlayerInit(Actor *this, const b2WorldId /*worldId*/, const KvList *params)
{
	this->showShadow = false;
	this->SignalHandler = SoundPlayerSignalHandler;
	SoundPlayerData *data = calloc(1, sizeof(SoundPlayerData));
	CheckAlloc(data);
	snprintf(data->asset, sizeof(data->asset), "audio/%s.gsnd", KvGetString(params, "sound", "sfx_click"));
	this->extraData = data;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void SoundPlayerDestroy(Actor *this)
{
	free(this->extraData);
}
