//
// Created by droc101 on 10/4/25.
//

#include "GameConfigLoader.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "../../../Structs/Asset.h"
#include "../AssetReader.h"
#include "../DataReader.h"
#include "../Error.h"

GameConfig config = {0};

void LoadGameConfig()
{
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
