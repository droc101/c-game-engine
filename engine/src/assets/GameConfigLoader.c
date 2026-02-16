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

#include "engine/helpers/Arguments.h"
#include "engine/structs/GlobalState.h"

GameConfig gameConfig = {0};

void LoadGameConfig()
{
	LogDebug("Loading game configuration...\n");

	if (HasCliArg("--game"))
	{
		const char *gameArg = GetCliArgStr("--game", "assets");
		SetAssetsPath(gameArg);
	} else
	{
		const size_t pathLen = strlen(GetState()->executableFolder) + strlen("assets") + 1;
		char *path = calloc(pathLen, 1);
		CheckAlloc(path);
		snprintf(path, pathLen, "%sassets", GetState()->executableFolder);
		SetAssetsPath(path);
		free(path);
	}

	Asset *asset = DecompressAsset("game.gkvl", false);
	if (!asset || asset->type != ASSET_TYPE_KV_LIST || asset->typeVersion != 1)
	{
		Error("Invalid game configuration");
	}
	size_t offset = 0;
	KvList configList = {};
	ReadKvList(asset->data, asset->size, &offset, configList);

	gameConfig.gameTitle = strdup(KvGetString(configList, "game_title", "Untitled"));
	gameConfig.gameCopyright = strdup(KvGetString(configList, "game_copyright", ""));
	gameConfig.discordAppId = KvGetUint64(configList, "discord_app_id", 0);

	KvListDestroy(configList);

	FreeAsset(asset);
}
