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

void LoadGameConfig();

#endif //GAME_GAMECONFIGLOADER_H
