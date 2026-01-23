//
// Created by droc101 on 8/4/25.
//

#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

/// Number of sound channels
/// (sounds that can play at the same time)
#define SOUND_SYSTEM_CHANNEL_COUNT 32

typedef enum SoundCategory SoundCategory;

typedef void (*SoundFinishedCallback)(void *data);

typedef struct SoundSystem SoundSystem;
typedef struct SoundChannel SoundChannel;
typedef struct SoundRequest SoundRequest;

extern SoundSystem soundSys;

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
	/// The function to call when the sound finishes
	SoundFinishedCallback completionCallback;
	/// Data to pass to @c completionCallback
	void *completionCallbackData;
	/// The sound category
	SoundCategory category;
	/// Whether to decode the sound fully ahead of time (false streams it instead)
	bool preload;
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
SoundChannel *PlaySound(const char *soundAsset, SoundCategory category);

/**
 * Attempt to play a sound
 * @param request The sound request
 * @return A SoundChannel handle, or NULL on failure
 * @warning If there are no free channels, the sound will not play.
 */
SoundChannel *PlaySoundEx(const SoundRequest *request);

/**
 * Pause a sound
 * @param effect The sound to pause
 * @warning Make sure to either stop or resume it at some point, or it will forever occupy a sound channel
 */
void PauseSound(const SoundChannel *effect);

/**
 * Resume a sound
 * @param effect The sound to resume
 */
void ResumeSound(const SoundChannel *effect);

/**
 * Stop a sound
 * @param effect The sound to stop
 */
void StopSound(const SoundChannel *effect);

#endif //SOUNDSYSTEM_H
