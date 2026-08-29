//
// Created by droc101 on 8/27/26.
//

#ifndef GAME_ACHIEVEMENTS_H
#define GAME_ACHIEVEMENTS_H

#include <engine/structs/List.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum StatType
{
	STAT_TYPE_INT,
	STAT_TYPE_FLOAT,
} StatType;

typedef union StatValue
{
	int32_t intValue;
	float floatValue;
} StatValue;

typedef struct Stat
{
	/// Value type of this stat
	StatType type;
	/// Default value of this stat
	StatValue defaultValue;
	/// Minimum value of this stat
	StatValue minValue;
	/// Maximum value of this stat
	StatValue maxValue;
	/// Whether this stat should persist across sessions, or reset to the default when the game is launched
	bool persist;

	/// The current value of this stat
	StatValue value;
} Stat;

typedef struct Achievement
{
	/// The icon to use when this achievement is locked
	const char *lockedIcon;
	/// The icon to use when this achievement is unlocked
	const char *unlockedIcon;
	/// The display name of this achievement
	const char *name;
	/// The description of this achievement
	const char *description;
	/// Whether this achievement should be hidden until it is unlocked
	bool hideUntilUnlocked;
	/// The key of the stat used to unlock this achievement
	const char *associatedStat;
	/// Value that will unlock this achievement when the associated stat is greater than or equal to it
	StatValue unlockThreshold;

	/// Whether this achievement is unlocked
	bool unlocked;
} Achievement;

typedef void (*RegisterGameAchievementsFunction)(void);

/**
 * Register an integer statistic
 * @param key The key referring to this statistic
 * @param defaultValue The default value of this statistic
 * @param minValue The minimum value of this statistic
 * @param maxValue The maximum value of this statistic
 * @param persist Whether this stat should persist across sessions, or reset to the default when the game is launched
 */
void RegisterIntegerStatistic(const char *key, int32_t defaultValue, int32_t minValue, int32_t maxValue, bool persist);
/**
 * Register a float statistic
 * @param key The key referring to this statistic
 * @param defaultValue The default value of this statistic
 * @param minValue The minimum value of this statistic
 * @param maxValue The maximum value of this statistic
 * @param persist Whether this stat should persist across sessions, or reset to the default when the game is launched
 */
void RegisterFloatStatistic(const char *key, float defaultValue, float minValue, float maxValue, bool persist);

/**
 * Register an achievement
 * @param key The key referring to this achievement
 * @param name The display name of this achievement
 * @param description The description of this achievement
 * @param hideUntilUnlocked Whether this achievement should be hidden until it is unlocked
 * @param lockedIcon The icon to use when this achievement is locked
 * @param unlockedIcon The icon to use when this achievement is unlocked
 */
void RegisterAchievement(const char *key,
						 const char *name,
						 const char *description,
						 bool hideUntilUnlocked,
						 const char *lockedIcon,
						 const char *unlockedIcon);
/**
 * Register an achievement tied to an integer statistic
 * @param key The key referring to this achievement
 * @param name The display name of this achievement
 * @param description The description of this achievement
 * @param hideUntilUnlocked Whether this achievement should be hidden until it is unlocked
 * @param lockedIcon The icon to use when this achievement is locked
 * @param unlockedIcon The icon to use when this achievement is unlocked
 * @param statName The key of the stat used to unlock this achievement
 * @param unlockThreshold The value that will unlock this achievement when the associated stat is greater than or equal to it
 */
void RegisterIntStatBasedAchievement(const char *key,
									 const char *name,
									 const char *description,
									 bool hideUntilUnlocked,
									 const char *lockedIcon,
									 const char *unlockedIcon,
									 const char *statName,
									 int32_t unlockThreshold);
/**
 * Register an achievement tied to a float statistic
 * @param key The key referring to this achievement
 * @param name The display name of this achievement
 * @param description The description of this achievement
 * @param hideUntilUnlocked Whether this achievement should be hidden until it is unlocked
 * @param lockedIcon The icon to use when this achievement is locked
 * @param unlockedIcon The icon to use when this achievement is unlocked
 * @param statName The key of the stat used to unlock this achievement
 * @param unlockThreshold The value that will unlock this achievement when the associated stat is greater than or equal to it
 */
void RegisterFloatStatBasedAchievement(const char *key,
									   const char *name,
									   const char *description,
									   bool hideUntilUnlocked,
									   const char *lockedIcon,
									   const char *unlockedIcon,
									   const char *statName,
									   float unlockThreshold);

/**
 * Initialize the achievement system
 * @param RegisterGameAchievements The function to call to register achievements
 */
void InitAchievementSystem(RegisterGameAchievementsFunction RegisterGameAchievements);

/**
 * Save achievements and statistics
 */
void SaveAchievements();

/**
 * Destroy the achievement system
 */
void DestroyAchievementSystem();

/**
 * Increment an integer statistic
 * @param stat The statistic to increment
 * @param change The value to increment by
 */
void IncrementIntegerStatistic(const char *stat, int32_t change);
/**
 * Increment a float statistic
 * @param stat The statistic to increment
 * @param change The value to increment by
 */
void IncrementFloatStatistic(const char *stat, float change);

/**
 * Set an integer statistic's value
 * @param stat The statistic to modify
 * @param newValue The value to set
 */
void SetIntegerStatistic(const char *stat, int32_t newValue);
/**
 * Set a float statistic's value
 * @param stat The statistic to modify
 * @param newValue The value to set
 */
void SetFloatStatistic(const char *stat, float newValue);

/**
 * Get the value of an integer statistic
 */
int32_t GetIntegerStatistic(const char *stat);
/**
 * Get the value of a float statistic
 */
float GetFloatStatistic(const char *stat);

/**
 * Unlock an achievement
 */
void UnlockAchievement(const char *ach);
/**
 * Lock an achievement
 */
void LockAchievement(const char *ach);

/**
 * Check if an achievement is unlocked
 */
bool IsAchievementUnlocked(const char *ach);

/**
 * Copy the key names of all achievements into a list
 * @note do not modify or free the keys
 */
void GetAchievementKeys(List *dest);
/**
 * Get the icon for an achievement, respecting its locked/unlocked state
 */
const char *GetAchievementIcon(const char *key);
/**
 * Get a raw achievement struct
 */
const Achievement *GetAchievement(const char *key);

/**
 * Reset all achievements and statistics
 */
void ResetAchievements();

#endif //GAME_ACHIEVEMENTS_H
