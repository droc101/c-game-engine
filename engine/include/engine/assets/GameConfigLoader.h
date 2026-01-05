//
// Created by droc101 on 10/4/25.
//

#ifndef GAME_GAMECONFIGLOADER_H
#define GAME_GAMECONFIGLOADER_H

#include <stddef.h>

typedef struct GameConfig GameConfig;

struct GameConfig
{
	/// The title of the game, used for the window title
	char *gameTitle;
	/// The copyright of the game
	char *gameCopyright;

	/// The Discord developer app ID, used for rich presence
	size_t discordAppId;
};

/// The loaded game config
extern GameConfig gameConfig;

/**
 * Load the game config asset
 */
void LoadGameConfig();

#endif //GAME_GAMECONFIGLOADER_H
