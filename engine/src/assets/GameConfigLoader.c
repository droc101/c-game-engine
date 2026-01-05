//
// Created by droc101 on 10/4/25.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/structs/Asset.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GameConfig gameConfig = {0};

void LoadGameConfig()
{
	LogDebug("Loading game configuration...\n");
	Asset *asset = DecompressAsset("game.game", false);
	if (!asset || asset->type != ASSET_TYPE_GAME_CONFIG || asset->typeVersion != 1)
	{
		Error("Invalid game configuration");
	}
	size_t offset = 0;
	gameConfig.gameTitle = ReadStringSafe(asset->data, &offset, asset->size, NULL);
	gameConfig.gameCopyright = ReadStringSafe(asset->data, &offset, asset->size, NULL);
	gameConfig.discordAppId = ReadSizeT(asset->data, &offset);

	FreeAsset(asset);
}
