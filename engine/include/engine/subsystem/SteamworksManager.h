//
// Created by droc101 on 8/20/26.
//

#ifndef GAME_STEAMWORKSMANAGER_H
#define GAME_STEAMWORKSMANAGER_H


#ifdef __cplusplus
extern "C" {
#endif

	bool InitSteamworks();

	bool IsSteamworksRunning();

	void ProcessSteamworks();

	void ShutdownSteamworks();

	void SteamworksTest();

#ifdef __cplusplus
}
#endif

#endif //GAME_STEAMWORKSMANAGER_H
