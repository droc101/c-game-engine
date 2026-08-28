//
// Created by droc101 on 8/27/26.
//

#include <engine/assets/AssetReader.h>
#include <engine/structs/Achievements.h>
#include <helpers/GameAchievements.h>
#include <stdbool.h>
#include <stdint.h>

void RegisterGameAchievements()
{
	RegisterIntegerStatistic(GAME_STAT_ENEMIES_KILLED, 0, 0, INT32_MAX, false);
	RegisterIntegerStatistic(GAME_STAT_COINS_COLLECTED, 0, 0, INT32_MAX, true);

	RegisterIntStatBasedAchievement(GAME_ACHIEVEMENT_KILL_FIFTY_ENEMIES,
									"meanie :(",
									"Erase 50 enemies in one session",
									true,
									TEXTURE("interface/achievement/" GAME_ACHIEVEMENT_KILL_FIFTY_ENEMIES "_locked"),
									TEXTURE("interface/achievement/" GAME_ACHIEVEMENT_KILL_FIFTY_ENEMIES "_unlocked"),
									GAME_STAT_ENEMIES_KILLED,
									50);
	RegisterIntStatBasedAchievement(GAME_ACHIEVEMENT_COLLECT_ONE_HUNDRED_COINS,
									"I'm Rich!",
									"Collect 100 coins",
									false,
									TEXTURE("interface/achievement/" GAME_ACHIEVEMENT_COLLECT_ONE_HUNDRED_COINS
											"_locked"),
									TEXTURE("interface/achievement/" GAME_ACHIEVEMENT_COLLECT_ONE_HUNDRED_COINS
											"_unlocked"),
									GAME_STAT_COINS_COLLECTED,
									100);
	RegisterAchievement(GAME_ACHIEVEMENT_GET_GOAL,
						"Winner",
						"Collect a goal",
						false,
						TEXTURE("interface/achievement/" GAME_ACHIEVEMENT_GET_GOAL "_locked"),
						TEXTURE("interface/achievement/" GAME_ACHIEVEMENT_GET_GOAL "_unlocked"));
}
