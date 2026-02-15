//
// Created by droc101 on 10/4/25.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/structs/Asset.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GameConfig gameConfig = {0};

AssetPath *CreateAssetPath(const AssetPathType type, const AssetPathFlags flags, char *path)
{
	AssetPath *assetPath = malloc(sizeof(AssetPath));
	CheckAlloc(assetPath);
	assetPath->flags = flags;
	assetPath->type = type;
	if (type == ABSOLUTE_PATH)
	{
		assetPath->path = strdup(path);
	} else if (type == RELATIVE_TO_EXECUTABLE_DIRECTORY)
	{
		const size_t pathLen = strlen(GetState()->executableFolder) + 1 + strlen(path) + 1;
		assetPath->path = malloc(pathLen);
		snprintf(assetPath->path, pathLen, "%s/%s", GetState()->executableFolder, path);
	}
	return assetPath;
}

void LoadGameConfig(const char *game)
{
	LogDebug("Loading game configuration...\n");
	char *configPath = NULL;
	if (IsPathAbsolute(game))
	{
		configPath = malloc(strlen(game) + strlen("/game.gkvl") + 1); // TODO use game.gcfg
		CheckAlloc(configPath);
		if (game[strlen(game) - 1] == '/')
		{
			sprintf(configPath, "%sgame.gkvl", game);
		} else
		{
			sprintf(configPath, "%s/game.gkvl", game);
		}
	} else
	{
		configPath = malloc(strlen(GetState()->executableFolder) + strlen(game) + 1 + strlen("game.gkvl") + 1);
		CheckAlloc(configPath);
		sprintf(configPath, "%s%s/game.gkvl", GetState()->executableFolder, game);
	}
	LogDebug("Loading game.gkvl from %s\n", configPath);
	FILE *file = fopen(configPath, "rb");
	free(configPath);
	if (!file)
	{
		Error("Failed to open game configuration");
	}
	Asset *asset = CreateAssetFromFile(file);
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

	ListInit(gameConfig.assetPaths, LIST_POINTER);

	ParamArray *searchPaths = KvGetArray(configList, "search_paths");
	if (!searchPaths)
	{
		Error("game.gkvl is missing the search_paths key!");
	}

	for (size_t i = 0; i < searchPaths->length; i++)
	{
		const Param *searchPathParam = &searchPaths->data[i];
		if (searchPathParam->type == PARAM_TYPE_KV_LIST)
		{
			const bool allowCodeExec = KvGetBool(searchPathParam->kvListValue, "allow_code_execution", false);
			const bool pathIsAbsolute = KvGetBool(searchPathParam->kvListValue, "path_is_absolute", false);
			char *searchPath = strdup(KvGetString(searchPathParam->kvListValue, "search_path", ""));
			const AssetPathType type = pathIsAbsolute ? ABSOLUTE_PATH : RELATIVE_TO_EXECUTABLE_DIRECTORY;
			AssetPathFlags flags = 0;
			if (allowCodeExec)
			{
				flags |= ASSET_PATH_ALLOW_CODE_EXECUTION;
			}
			ListAdd(gameConfig.assetPaths, CreateAssetPath(type, flags, searchPath));
		} else if (searchPathParam->type == PARAM_TYPE_STRING)
		{
			ListAdd(gameConfig.assetPaths,
					CreateAssetPath(RELATIVE_TO_EXECUTABLE_DIRECTORY, 0, searchPathParam->stringValue));
		} else
		{
			Error("Invalid search path in game.gkvl");
		}
	}

	if (gameConfig.assetPaths.length == 0)
	{
		Error("game.gkvl did not provide any asset paths!");
	}

	KvListDestroy(configList);

	FreeAsset(asset);
}

void DestroyGameConfig()
{
	LogDebug("Destroying game config...\n");
	for (size_t i = 0; i < gameConfig.assetPaths.length; i++)
	{
		AssetPath *assetPath = ListGetPointer(gameConfig.assetPaths, i);
		free(assetPath->path);
		free(assetPath);
	}
	ListFree(gameConfig.assetPaths);
}
