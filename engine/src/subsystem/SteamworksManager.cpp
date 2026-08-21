//
// Created by droc101 on 8/20/26.
//

// NOLINTBEGIN(*-pro-type-vararg)

#include <engine/subsystem/SteamworksManager.h>

#ifdef ENABLE_STEAMWORKS
#include <steam/isteamfriends.h>
#include <steam/isteamuser.h>
#include <steam/steam_api.h>
#include <steam/steam_api_common.h>
#endif

extern "C" {
#include <engine/subsystem/Logging.h>
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

void SteamworksTest()
{
#ifdef ENABLE_STEAMWORKS
	if (steamworksRunning)
	{
		SteamFriends()->ActivateGameOverlay("friends");
		LogDebug("You are %s, Steam level %d\n", SteamFriends()->GetPersonaName(), SteamUser()->GetPlayerSteamLevel());
	}
#endif
}

// NOLINTEND(*-pro-type-vararg)
