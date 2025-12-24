//
// Created by droc101 on 10/4/25.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/structs/Asset.h>
#include <engine/structs/GlobalState.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GameConfig config = {0};

void LoadGameConfig(const char *game)
{
	LogDebug("Loading game configuration...\n");
	char *configPath = NULL;
	if (IsPathAbsolute(game))
	{
		configPath = malloc(strlen(game) + strlen("/game.game") + 1); // TODO use game.gcfg
		CheckAlloc(configPath);
		if (game[strlen(game) - 1] == '/')
		{
			sprintf(configPath, "%sgame.game", game);
		} else
		{
			sprintf(configPath, "%s/game.game", game);
		}
	} else
	{
		configPath = malloc(strlen(GetState()->executableFolder) + strlen(game) + 1 + strlen("game.game") + 1);
		CheckAlloc(configPath);
		sprintf(configPath, "%s%s/game.game", GetState()->executableFolder, game);
	}
	LogDebug("Loading game.game from %s\n", configPath);
	FILE *file = fopen(configPath, "rb");
	free(configPath);
	if (!file)
	{
		Error("Failed to open game configuration");
	}
	Asset *asset = CreateAssetFromFile(file);
	if (!asset || asset->type != ASSET_TYPE_GAME_CONFIG || asset->typeVersion != 1)
	{
		Error("Invalid game configuration");
	}
	size_t offset = 0;
	config.gameTitle = ReadStringSafe(asset->data, &offset, asset->size, NULL);
	config.gameCopyright = ReadStringSafe(asset->data, &offset, asset->size, NULL);
	config.discordAppId = ReadSizeT(asset->data, &offset);

	FreeAsset(asset);
}
