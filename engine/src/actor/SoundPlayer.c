//
// Created by droc101 on 5/29/25.
//

#include <engine/actor/SoundPlayer.h>
#include <engine/assets/AssetReader.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/SoundSystem.h>
#include <joltc/Math/RVec3.h>
#include <joltc/Math/Transform.h>
#include <joltc/Physics/Body/BodyInterface.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SoundPlayerData
{
	char *asset; // asset name of the sound effect to play
	SoundChannel *effect;
	int loops;
	float volume;
	bool preload;
	SoundCategory category;
	bool positional;
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
		StopSound(data->effect);
	}
	free(data->asset);
}

static void SoundPlayerPlayHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	JPH_RVec3 position;
	JPH_BodyInterface_GetPosition(this->bodyInterface, this->bodyId, &position);
	SoundPlayerData *data = this->extraData;
	const SoundRequest request = {
		.soundAsset = data->asset,
		.category = data->category,
		.volume = data->volume,
		.completionCallback = SoundPlayerSoundDone,
		.completionCallbackData = data,
		.numLoops = data->loops,
		.preload = data->preload,
								  .positional = data->positional,
								  .position = {.x = position.x, .y = position.y, .z = position.z}};
	data->effect = PlaySoundEx(&request);
}

static void SoundPlayerPauseHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const SoundPlayerData *data = this->extraData;
	PauseSound(data->effect);
}

static void SoundPlayerResumeHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const SoundPlayerData *data = this->extraData;
	ResumeSound(data->effect);
}

static void SoundPlayerStopHandler(Actor *this, const Actor * /*sender*/, const Param * /*param*/)
{
	const SoundPlayerData *data = this->extraData;
	StopSound(data->effect);
}

void SoundPlayerInit(Actor *this, const KvList params, Transform * /*transform*/)
{
	SoundPlayerData *data = calloc(1, sizeof(SoundPlayerData));
	CheckAlloc(data);
	data->effect = NULL;
	const char *soundAsset = KvGetString(params, "sound", "sfx/click");
	data->asset = malloc(strlen(SOUND("")) + strlen(soundAsset) + 1);
	sprintf(data->asset, SOUND("%s"), soundAsset);
	data->loops = KvGetInt(params, "loops", 0);
	data->volume = KvGetFloat(params, "volume", 1);
	data->preload = KvGetBool(params, "preload", false);
	data->category = KvGetByte(params, "category", SOUND_CATEGORY_SFX);
	data->positional = false; //KvGetBool(params, "positional", false);
	this->extraData = data;
}

ActorDefinition soundPlayerActorDefinition = {
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
	RegisterDefaultActorInputs(&soundPlayerActorDefinition);
	RegisterActorInput(&soundPlayerActorDefinition, SOUND_PLAYER_INPUT_PLAY, SoundPlayerPlayHandler);
	RegisterActorInput(&soundPlayerActorDefinition, SOUND_PLAYER_INPUT_PAUSE, SoundPlayerPauseHandler);
	RegisterActorInput(&soundPlayerActorDefinition, SOUND_PLAYER_INPUT_RESUME, SoundPlayerResumeHandler);
	RegisterActorInput(&soundPlayerActorDefinition, SOUND_PLAYER_INPUT_STOP, SoundPlayerStopHandler);
	RegisterActor(SOUND_PLAYER_ACTOR_NAME, &soundPlayerActorDefinition);
}
