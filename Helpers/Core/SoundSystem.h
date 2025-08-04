//
// Created by droc101 on 8/4/25.
//

#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

/// Number of sound effect channels
/// (sound effects that can play at the same time)
#define SFX_CHANNEL_COUNT 32

typedef void(*SoundFinishedCallback)(void *data);

typedef struct SoundSystem SoundSystem;
typedef struct SoundEffect SoundEffect;

extern SoundSystem soundSys;

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
 * Change the bgm
 * @param asset Asset to change to
 */
void ChangeMusic(const char *asset);

/**
 * Stop the bgm
 */
void StopMusic();

/**
 * Attempt to play a sound effect
 * @param asset Sound effect to play
 * @param loops How many times to loop the sound, playing @code loops + 1@endcode times in total (or -1 to loop "forever")
 * @param volume The volume of the sound (float 0 - 1)
 * @param callback A function to call when the sound finishes (or NULL to not call one)
 * @param callbackData Data to pass to the callback function (ignored if callback is NULL)
 * @return A SoundEffect handle, or NULL on failure
 * @warning If there are no free channels, the sound effect will not play.
 */
SoundEffect *PlaySoundEffect(const char *asset,
							 int loops,
							 float volume,
							 SoundFinishedCallback callback,
							 void *callbackData);

/**
 * Pause a sound effect
 * @param effect The effect to pause
 * @warning Make sure to either stop or resume it at some point or it will forever occupy a sound channel
 */
void PauseSoundEffect(const SoundEffect *effect);

/**
 * Resume a sound effect
 * @param effect The effect to resume
 */
void ResumeSoundEffect(const SoundEffect *effect);

/**
 * Stop a sound effect
 * @param effect The effect to stop
 */
void StopSoundEffect(const SoundEffect *effect);

#endif //SOUNDSYSTEM_H
