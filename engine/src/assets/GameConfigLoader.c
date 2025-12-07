//
// Created by droc101 on 10/4/25.
//

#include <engine/assets/GameConfigLoader.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <engine/structs/Asset.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>

GameConfig config = {0};

void LoadGameConfig()
{
	LogDebug("Loading game configuration...\n");
	Asset *asset = DecompressAsset("game.game", false);
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
