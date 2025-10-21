//
// Created by droc101 on 5/29/25.
//

#include <engine/actor/SoundPlayer.h>
#include <engine/assets/AssetReader.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Param.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/SoundSystem.h>
#include <joltc/Math/Transform.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

static void SoundPlayerPlayHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	SoundPlayerData *data = this->extraData;
	data->effect = PlaySoundEffect(data->asset, data->loops, data->volume, SoundPlayerSoundDone, data);
}

static void SoundPlayerPauseHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const SoundPlayerData *data = this->extraData;
	PauseSoundEffect(data->effect);
}

static void SoundPlayerResumeHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const SoundPlayerData *data = this->extraData;
	ResumeSoundEffect(data->effect);
}

static void SoundPlayerStopHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const SoundPlayerData *data = this->extraData;
	StopSoundEffect(data->effect);
}

void SoundPlayerInit(Actor *this, const KvList params, Transform * /*transform*/)
{
	SoundPlayerData *data = calloc(1, sizeof(SoundPlayerData));
	CheckAlloc(data);
	data->effect = NULL;
	snprintf(data->asset, sizeof(data->asset), SOUND("%s"), KvGetString(params, "sound", "sfx/click"));
	data->loops = KvGetInt(params, "loops", 0);
	data->volume = KvGetFloat(params, "volume", 1);
	this->extraData = data;
}

static ActorDefinition definition = {
	.actorType = ACTOR_TYPE_SOUND_PLAYER,
	.Update = DefaultActorUpdate,
	.OnPlayerContactAdded = DefaultActorOnPlayerContactAdded,
	.OnPlayerContactPersisted = DefaultActorOnPlayerContactPersisted,
	.OnPlayerContactRemoved = DefaultActorOnPlayerContactRemoved,
	.RenderUi = DefaultActorRenderUi,
	.Destroy = SoundPlayerDestroy,
	.Init = SoundPlayerInit,
};

void RegisterSoundPlayer()
{
	RegisterDefaultActorInputs(&definition);
	RegisterActorInput(&definition, SOUND_PLAYER_INPUT_PLAY, SoundPlayerPlayHandler);
	RegisterActorInput(&definition, SOUND_PLAYER_INPUT_PAUSE, SoundPlayerPauseHandler);
	RegisterActorInput(&definition, SOUND_PLAYER_INPUT_RESUME, SoundPlayerResumeHandler);
	RegisterActorInput(&definition, SOUND_PLAYER_INPUT_STOP, SoundPlayerStopHandler);
	RegisterActor(SOUND_PLAYER_ACTOR_NAME, &definition);
}
