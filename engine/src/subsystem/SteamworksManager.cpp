//
// Created by droc101 on 8/20/26.
//

// NOLINTBEGIN(*-pro-type-vararg)

#include <engine/subsystem/SteamworksManager.h>

#ifdef ENABLE_STEAMWORKS
#include <steam/isteamuserstats.h>
#include <steam/steam_api.h>
#include <steam/steam_api_common.h>
#endif

extern "C" {
#include <engine/subsystem/Logging.h>
#include <stdint.h>
#include <stdlib.h>
}

namespace
{
	class SteamworksCallbackListener
	{
#ifdef ENABLE_STEAMWORKS
		// put STEAM_CALLBACK things here, make sure to `#ifdef ENABLE_STEAMWORKS` the implementations as well
#endif
	};
} // namespace

static SteamworksCallbackListener *callbackListener = nullptr;
static bool steamworksRunning = false;

bool InitSteamworks()
{
#ifdef ENABLE_STEAMWORKS
	if (SteamAPI_RestartAppIfNecessary(STEAMWORKS_APP_ID))
	{
		LogWarning("Restarting via Steam, this process will now exit.\n");
		exit(0);
	}
	SteamErrMsg err;
	const ESteamAPIInitResult initResult = SteamAPI_InitEx(&err);
	steamworksRunning = initResult == k_ESteamAPIInitResult_OK;
	if (!steamworksRunning)
	{
		LogError("Failed to init Steamworks: %s\n", err);
	}

	callbackListener = new SteamworksCallbackListener();

	return steamworksRunning;
#else
	return true;
#endif
}

bool IsSteamworksRunning()
{
	return steamworksRunning;
}

void ProcessSteamworks()
{
#ifdef ENABLE_STEAMWORKS
	if (steamworksRunning)
	{
		SteamAPI_RunCallbacks();
	}
#endif
}

void ShutdownSteamworks()
{
#ifdef ENABLE_STEAMWORKS
	if (steamworksRunning)
	{
		delete callbackListener;
		SteamAPI_Shutdown();
	}
#endif
}

bool IsSteamAchievementUnlocked(const char *achievement)
{
#ifdef ENABLE_STEAMWORKS
	if (steamworksRunning)
	{
		bool unlocked = false;
		if (SteamUserStats()->GetAchievement(achievement, &unlocked))
		{
			return unlocked;
		} else
		{
			LogError("Failed to get achievement \"%s\" from Steam, does it exist?\n", achievement);
		}
	}
#endif
	return false;
}

int32_t GetSteamIntegerStatistic(const char *statistic)
{
#ifdef ENABLE_STEAMWORKS
	if (steamworksRunning)
	{
		int32_t value = 0;
		if (SteamUserStats()->GetStat(statistic, &value))
		{
			return value;
		} else
		{
			LogError("Failed to get stat \"%s\" from Steam, does it exist?\n", statistic);
		}
	}
#endif
	return 0;
}

float GetSteamFloatStatistic(const char *statistic)
{
#ifdef ENABLE_STEAMWORKS
	if (steamworksRunning)
	{
		float value = 0;
		if (SteamUserStats()->GetStat(statistic, &value))
		{
			return value;
		} else
		{
			LogError("Failed to get stat \"%s\" from Steam, does it exist?\n", statistic);
		}
	}
#endif
	return 0.0f;
}

void SetSteamAchievementUnlocked(const char *achievement, bool unlocked)
{
#ifdef ENABLE_STEAMWORKS
	if (steamworksRunning)
	{
		if (unlocked)
		{
			if (!SteamUserStats()->SetAchievement(achievement))
			{
				LogError("Failed to unlock Steam achievement \"%s\", does it exist?\n", achievement);
			}
		} else
		{
			if (!SteamUserStats()->ClearAchievement(achievement))
			{
				LogError("Failed to lock Steam achievement \"%s\", does it exist?\n", achievement);
			}
		}
	}
#endif
}

void SetSteamIntegerStatistic(const char *statistic, const int32_t value)
{
#ifdef ENABLE_STEAMWORKS
	if (steamworksRunning)
	{
		if (!SteamUserStats()->SetStat(statistic, value))
		{
			LogError("Failed to set Steam stat \"%s\", does it exist?\n", statistic);
		}
	}
#endif
}

void SetSteamFloatStatistic(const char *statistic, const float value)
{
#ifdef ENABLE_STEAMWORKS
	if (steamworksRunning)
	{
		if (!SteamUserStats()->SetStat(statistic, value))
		{
			LogError("Failed to set Steam stat \"%s\", does it exist?\n", statistic);
		}
	}
#endif
}

void SaveSteamAchievementsAndStats()
{
#ifdef ENABLE_STEAMWORKS
	if (steamworksRunning)
	{
		if (!SteamUserStats()->StoreStats())
		{
			LogError("failed to store Steam achievements & stats\n");
		}
	}
#endif
}

void ResetSteamAchievementsAndStats()
{
#ifdef ENABLE_STEAMWORKS
	if (steamworksRunning)
	{
		if (!SteamUserStats()->ResetAllStats(true))
		{
			LogError("failed to reset Steam achievements & stats\n");
		}
	}
#endif
}

// NOLINTEND(*-pro-type-vararg)
