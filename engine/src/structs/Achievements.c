//
// Created by droc101 on 8/27/26.
//

#include <engine/assets/KvlFile.h>
#include <engine/helpers/MathEx.h>
#include <engine/structs/Achievements.h>
#include <engine/structs/Dict.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <m-core.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define ACHIEVEMENTS_FILE "achievements.kvl"

DEFINE_DICT(StatDict, const char *, M_CSTR_OPLIST, Stat *, M_PTR_OPLIST);
DEFINE_DICT(AchievementDict, const char *, M_CSTR_OPLIST, Achievement *, M_PTR_OPLIST);

static StatDict statistics;
static AchievementDict achievements;

void RegisterIntegerStatistic(const char *key,
							  const int32_t defaultValue,
							  const int32_t minValue,
							  const int32_t maxValue,
							  const bool persist)
{
	Stat *stat = malloc(sizeof(Stat));
	CheckAlloc(stat);
	stat->type = STAT_TYPE_INT;
	stat->defaultValue.intValue = defaultValue;
	stat->minValue.intValue = minValue;
	stat->maxValue.intValue = maxValue;
	stat->persist = persist;

	stat->value.intValue = defaultValue;

	StatDict_set_at(statistics, key, stat);
}

void RegisterFloatStatistic(const char *key,
							const float defaultValue,
							const float minValue,
							const float maxValue,
							const bool persist)
{
	Stat *stat = malloc(sizeof(Stat));
	CheckAlloc(stat);
	stat->type = STAT_TYPE_FLOAT;
	stat->defaultValue.floatValue = defaultValue;
	stat->minValue.floatValue = minValue;
	stat->maxValue.floatValue = maxValue;
	stat->persist = persist;

	stat->value.floatValue = defaultValue;

	StatDict_set_at(statistics, key, stat);
}

void RegisterAchievement(const char *key,
						 const char *name,
						 const char *description,
						 const bool hideUntilUnlocked,
						 const char *lockedIcon,
						 const char *unlockedIcon)
{
	Achievement *ach = malloc(sizeof(Achievement));
	CheckAlloc(ach);
	ach->name = name;
	ach->description = description;
	ach->hideUntilUnlocked = hideUntilUnlocked;
	ach->lockedIcon = lockedIcon;
	ach->unlockedIcon = unlockedIcon;
	ach->associatedStat = NULL;

	ach->unlocked = false;

	AchievementDict_set_at(achievements, key, ach);
}

void RegisterIntStatBasedAchievement(const char *key,
									 const char *name,
									 const char *description,
									 const bool hideUntilUnlocked,
									 const char *lockedIcon,
									 const char *unlockedIcon,
									 const char *statName,
									 const int32_t unlockThreshold)
{
	Stat **st = StatDict_safe_get(statistics, statName);
	if (!st)
	{
		LogError("Failed to register achievement \"%s\" because it referenced a nonexistent stat \"%s\".",
				 key,
				 statName);
		return;
	}
	if ((*st)->type != STAT_TYPE_INT)
	{
		LogError("Failed to register achievement \"%s\" because it referenced stat \"%s\" with an incompatible type.",
				 key,
				 statName);
		return;
	}

	Achievement *ach = malloc(sizeof(Achievement));
	CheckAlloc(ach);
	ach->name = name;
	ach->description = description;
	ach->hideUntilUnlocked = hideUntilUnlocked;
	ach->lockedIcon = lockedIcon;
	ach->unlockedIcon = unlockedIcon;
	ach->associatedStat = statName;
	ach->unlockThreshold.intValue = unlockThreshold;

	ach->unlocked = false;

	AchievementDict_set_at(achievements, key, ach);
}


void RegisterFloatStatBasedAchievement(const char *key,
									   const char *name,
									   const char *description,
									   const bool hideUntilUnlocked,
									   const char *lockedIcon,
									   const char *unlockedIcon,
									   const char *statName,
									   const float unlockThreshold)
{
	Stat **st = StatDict_safe_get(statistics, statName);
	if (!st)
	{
		LogError("Failed to register achievement \"%s\" because it referenced a nonexistent stat \"%s\".",
				 key,
				 statName);
		return;
	}
	if ((*st)->type != STAT_TYPE_FLOAT)
	{
		LogError("Failed to register achievement \"%s\" because it referenced stat \"%s\" with an incompatible type.",
				 key,
				 statName);
		return;
	}

	Achievement *ach = malloc(sizeof(Achievement));
	CheckAlloc(ach);
	ach->name = name;
	ach->description = description;
	ach->hideUntilUnlocked = hideUntilUnlocked;
	ach->lockedIcon = lockedIcon;
	ach->unlockedIcon = unlockedIcon;
	ach->associatedStat = statName;
	ach->unlockThreshold.floatValue = unlockThreshold;

	ach->unlocked = false;

	AchievementDict_set_at(achievements, key, ach);
}

void InitAchievementSystem(const RegisterGameAchievementsFunction RegisterGameAchievements)
{
	StatDict_init(statistics);
	AchievementDict_init(achievements);

	RegisterGameAchievements();

	KvList list;
	if (ReadKvlFile(ACHIEVEMENTS_FILE, list))
	{
		KvList savedStats;
		if (KvGetList(list, "statistics", savedStats))
		{
			StatDict_iterator it;
			StatDict_it(it, statistics);
			while (!StatDict_end_p(it))
			{
				const StatDict_pair *pair = StatDict_ref(it);

				if (!pair->value->persist)
				{
					StatDict_next(it);
					continue;
				}

				switch (pair->value->type)
				{
					case STAT_TYPE_INT:
						pair->value->value.intValue = KvGetInt(savedStats,
															   pair->key,
															   pair->value->defaultValue.intValue);
						break;
					case STAT_TYPE_FLOAT:
						pair->value->value.floatValue = KvGetFloat(savedStats,
																   pair->key,
																   pair->value->defaultValue.floatValue);
						break;
				}
				StatDict_next(it);
			}
			KvListDestroy(savedStats);
		}

		KvList savedAchievements;
		if (KvGetList(list, "achievements", savedAchievements))
		{
			AchievementDict_iterator it;
			AchievementDict_it(it, achievements);
			while (!AchievementDict_end_p(it))
			{
				const AchievementDict_pair *pair = AchievementDict_ref(it);
				pair->value->unlocked = KvGetBool(savedAchievements, pair->key, false);
				AchievementDict_next(it);
			}
			KvListDestroy(savedAchievements);
		}

		KvListDestroy(list);
	}
}

void SaveAchievements()
{
	KvList list;
	KvListCreate(list);

	KvList savedStats;
	KvListCreate(savedStats);
	StatDict_iterator stat_it;
	StatDict_it(stat_it, statistics);
	while (!StatDict_end_p(stat_it))
	{
		const StatDict_pair *pair = StatDict_ref(stat_it);

		if (!pair->value->persist)
		{
			StatDict_next(stat_it);
			continue;
		}

		switch (pair->value->type)
		{
			case STAT_TYPE_INT:
				KvSetInt(savedStats, pair->key, pair->value->value.intValue);
				break;
			case STAT_TYPE_FLOAT:
				KvSetFloat(savedStats, pair->key, pair->value->value.floatValue);
				break;
		}
		StatDict_next(stat_it);
	}
	KvSetList(list, "statistics", savedStats);

	KvList savedAchievements;
	KvListCreate(savedAchievements);
	AchievementDict_iterator ach_it;
	AchievementDict_it(ach_it, achievements);
	while (!AchievementDict_end_p(ach_it))
	{
		const AchievementDict_pair *pair = AchievementDict_ref(ach_it);
		KvSetBool(savedAchievements, pair->key, pair->value->unlocked);
		AchievementDict_next(ach_it);
	}
	KvSetList(list, "achievements", savedAchievements);


	if (!WriteKvlFile(ACHIEVEMENTS_FILE, list))
	{
		LogError("Failed to save achievements!\n");
	}

	KvListDestroy(savedStats);
	KvListDestroy(savedAchievements);
}

void DestroyAchievementSystem()
{
	SaveAchievements();

	StatDict_iterator stat_it;
	StatDict_it(stat_it, statistics);
	while (!StatDict_end_p(stat_it))
	{
		const StatDict_pair *pair = StatDict_ref(stat_it);
		free(pair->value);
		StatDict_next(stat_it);
	}
	StatDict_clear(statistics);

	AchievementDict_iterator ach_it;
	AchievementDict_it(ach_it, achievements);
	while (!AchievementDict_end_p(ach_it))
	{
		const AchievementDict_pair *pair = AchievementDict_ref(ach_it);
		free(pair->value);
		AchievementDict_next(ach_it);
	}
	AchievementDict_clear(achievements);
}

static void UnlockAchievementsBasedOnStat(const char *statKey, const Stat *stat)
{
	AchievementDict_iterator ach_it;
	AchievementDict_it(ach_it, achievements);
	while (!AchievementDict_end_p(ach_it))
	{
		const AchievementDict_pair *pair = AchievementDict_ref(ach_it);
		const Achievement *ach = pair->value;
		if (!ach->associatedStat || ach->unlocked)
		{
			AchievementDict_next(ach_it);
			continue;
		}
		if (strcmp(ach->associatedStat, statKey) == 0)
		{
			bool unlock = false;
			switch (stat->type)
			{
				case STAT_TYPE_INT:
					unlock = stat->value.intValue >= ach->unlockThreshold.intValue;
					break;
				case STAT_TYPE_FLOAT:
					unlock = stat->value.floatValue >= ach->unlockThreshold.floatValue;
					break;
			}
			if (unlock)
			{
				UnlockAchievement(pair->key);
			}
		}
		AchievementDict_next(ach_it);
	}
}

void IncrementIntegerStatistic(const char *stat, const int32_t change)
{
	Stat *statistic = *StatDict_get(statistics, stat);
	statistic->value.intValue = clamp(statistic->value.intValue + change,
									  statistic->minValue.intValue,
									  statistic->maxValue.intValue);
	UnlockAchievementsBasedOnStat(stat, statistic);
}
void IncrementFloatStatistic(const char *stat, const float change)
{
	Stat *statistic = *StatDict_get(statistics, stat);
	statistic->value.floatValue = clamp(statistic->value.floatValue + change,
										statistic->minValue.floatValue,
										statistic->maxValue.floatValue);
	UnlockAchievementsBasedOnStat(stat, statistic);
}

void SetIntegerStatistic(const char *stat, const int32_t newValue)
{
	Stat *statistic = *StatDict_get(statistics, stat);
	statistic->value.intValue = clamp(newValue, statistic->minValue.intValue, statistic->maxValue.intValue);
	UnlockAchievementsBasedOnStat(stat, statistic);
}
void SetFloatStatistic(const char *stat, const float newValue)
{
	Stat *statistic = *StatDict_get(statistics, stat);
	statistic->value.floatValue = clamp(newValue, statistic->minValue.floatValue, statistic->maxValue.floatValue);
	UnlockAchievementsBasedOnStat(stat, statistic);
}

int32_t GetIntegerStatistic(const char *stat)
{
	return (*StatDict_get(statistics, stat))->value.intValue;
}
float GetFloatStatistic(const char *stat)
{
	return (*StatDict_get(statistics, stat))->value.floatValue;
}

void UnlockAchievement(const char *ach)
{
	Achievement *a = *AchievementDict_get(achievements, ach);
	if (!a->unlocked)
	{
		LogInfo("Unlocked \"%s\"!", a->name); // TODO some sort of callback so there can be a UI
		a->unlocked = true;
		SaveAchievements();
	}
}
void LockAchievement(const char *ach)
{
	Achievement *a = *AchievementDict_get(achievements, ach);
	if (a->unlocked)
	{
		a->unlocked = false;
		SaveAchievements();
	}
}

bool IsAchievementUnlocked(const char *ach)
{
	return (*AchievementDict_get(achievements, ach))->unlocked;
}

void GetAchievementKeys(List *dest)
{
	AchievementDict_iterator ach_it;
	AchievementDict_it(ach_it, achievements);
	while (!AchievementDict_end_p(ach_it))
	{
		const AchievementDict_pair *pair = AchievementDict_ref(ach_it);
		ListAdd(*dest, pair->key);
		AchievementDict_next(ach_it);
	}
}


const char *GetAchievementIcon(const char *key)
{
	const Achievement *a = *AchievementDict_get(achievements, key);
	return a->unlocked ? a->unlockedIcon : a->lockedIcon;
}

const Achievement *GetAchievement(const char *key)
{
	return *AchievementDict_get(achievements, key);
}
