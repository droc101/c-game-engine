//
// Created by droc101 on 10/4/25.
//

#ifndef GAME_GAMECONFIGLOADER_H
#define GAME_GAMECONFIGLOADER_H

#include <stddef.h>

typedef struct GameConfig GameConfig;

struct GameConfig
{
	char *gameTitle;
	char *gameCopyright;

	size_t discordAppId;
};

extern GameConfig config;

/**
 * Load the game config asset
 */
void LoadGameConfig(const char *game);

#endif //GAME_GAMECONFIGLOADER_H
