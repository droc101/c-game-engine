//
// Created by droc101 on 8/20/26.
//

#ifndef GAME_STEAMWORKSMANAGER_H
#define GAME_STEAMWORKSMANAGER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//region General
	bool InitSteamworks();

	bool IsSteamworksRunning();

	void ProcessSteamworks();

	void ShutdownSteamworks();
//endregion

	//region Achievements & Stats
	bool IsSteamAchievementUnlocked(const char *achievement);
	int32_t GetSteamIntegerStatistic(const char *statistic);
	float GetSteamFloatStatistic(const char *statistic);

	void SetSteamAchievementUnlocked(const char *achievement, bool unlocked);
	void SetSteamIntegerStatistic(const char *statistic, int32_t value);
	void SetSteamFloatStatistic(const char *statistic, float value);

	void SaveSteamAchievementsAndStats();

	void ResetSteamAchievementsAndStats();
	//endregion

#ifdef __cplusplus
}
#endif

#endif //GAME_STEAMWORKSMANAGER_H
