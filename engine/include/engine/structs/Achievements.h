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
	StatType type;
	StatValue defaultValue;
	StatValue minValue;
	StatValue maxValue;
	bool persist;

	StatValue value;
} Stat;

typedef struct Achievement
{
	const char *lockedIcon;
	const char *unlockedIcon;
	const char *name;
	const char *description;
	bool hideUntilUnlocked;
	const char *associatedStat;
	StatValue unlockThreshold;

	bool unlocked;
} Achievement;

typedef void (*RegisterGameAchievementsFunction)(void);

void RegisterIntegerStatistic(const char *key, int32_t defaultValue, int32_t minValue, int32_t maxValue, bool persist);
void RegisterFloatStatistic(const char *key, float defaultValue, float minValue, float maxValue, bool persist);

void RegisterAchievement(const char *key,
						 const char *name,
						 const char *description,
						 bool hideUntilUnlocked,
						 const char *lockedIcon,
						 const char *unlockedIcon);
void RegisterIntStatBasedAchievement(const char *key,
									 const char *name,
									 const char *description,
									 bool hideUntilUnlocked,
									 const char *lockedIcon,
									 const char *unlockedIcon,
									 const char *statName,
									 int32_t unlockThreshold);
void RegisterFloatStatBasedAchievement(const char *key,
									   const char *name,
									   const char *description,
									   bool hideUntilUnlocked,
									   const char *lockedIcon,
									   const char *unlockedIcon,
									   const char *statName,
									   float unlockThreshold);

void InitAchievementSystem(RegisterGameAchievementsFunction RegisterGameAchievements);

void SaveAchievements();

void DestroyAchievementSystem();

void IncrementIntegerStatistic(const char *stat, int32_t change);
void IncrementFloatStatistic(const char *stat, float change);

void SetIntegerStatistic(const char *stat, int32_t newValue);
void SetFloatStatistic(const char *stat, float newValue);

int32_t GetIntegerStatistic(const char *stat);
float GetFloatStatistic(const char *stat);

void UnlockAchievement(const char *ach);
void LockAchievement(const char *ach);

bool IsAchievementUnlocked(const char *ach);

void GetAchievementKeys(List *dest);
const char *GetAchievementIcon(const char *key);
const Achievement *GetAchievement(const char *key);

#endif //GAME_ACHIEVEMENTS_H
