//
// Created by droc101 on 8/4/25.
//

#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

/// Number of sound channels
/// (sounds that can play at the same time)
#define SOUND_SYSTEM_CHANNEL_COUNT 32

#define SOUND_SYSTEM_INVALID_HANDLE UINT64_MAX

typedef enum SoundCategory SoundCategory;

typedef void (*SoundFinishedCallback)(void *data);

typedef struct SoundRequest SoundRequest;

typedef uint64_t SoundHandle;

enum SoundCategory
{
	SOUND_CATEGORY_MUSIC,
	SOUND_CATEGORY_SFX,
	SOUND_CATEGORY_UI,
};

struct SoundRequest
{
	/// The path to the sound asset to play
	const char *soundAsset;

	/// The number of times to loop the sound, -1 to loop forever
	int64_t numLoops;
	/// The volume of this sound
	float volume;
	/// Whether to decode the sound fully ahead of time (false streams it instead)
	bool preload;
	/// The sound category
	SoundCategory category;

	/// The function to call when the sound finishes
	SoundFinishedCallback completionCallback;
	/// Data to pass to @c completionCallback
	void *completionCallbackData;

	/// Whether this sound should be positional
	bool positional;
	/// The 3D position of this sound
	Vector3 position;
};

/**
 * Set up the sound system
 */
void InitSoundSystem();

/**
 * Clean up the sound system
 */
void DestroySoundSystem();

/**
 * Update the sound system
 */
void UpdateSoundSystem();

/**
 * Update the volume from the options
 * @note This function should be called whenever the options are changed
 */
void UpdateVolume();

/**
 * Attempt to play a sound
 * @param soundAsset The path to the sound asset
 * @param category The sound category
 * @return A SoundChannel handle, or NULL on failure
 * @warning If there are no free channels, the sound will not play.
 */
SoundHandle PlaySound(const char *soundAsset, SoundCategory category);

/**
 * Attempt to play a sound
 * @param request The sound request
 * @return A SoundChannel handle, or NULL on failure
 * @warning If there are no free channels, the sound will not play.
 */
SoundHandle PlaySoundEx(const SoundRequest *request);

/**
 * Pause a sound
 * @param effect The sound to pause
 * @warning Make sure to either stop or resume it at some point, or it will forever occupy a sound channel
 */
void PauseSound(SoundHandle effect);

/**
 * Resume a sound
 * @param effect The sound to resume
 */
void ResumeSound(SoundHandle effect);

/**
 * Stop a sound
 * @param effect The sound to stop
 */
void StopSound(SoundHandle effect);

/**
 * Immediately stop all sounds
 */
void StopAllSounds();

void DPrintSoundSystem();

#endif //SOUNDSYSTEM_H
