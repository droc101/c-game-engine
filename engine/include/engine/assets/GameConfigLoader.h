//
// Created by droc101 on 10/4/25.
//

#ifndef GAME_GAMECONFIGLOADER_H
#define GAME_GAMECONFIGLOADER_H

#include <engine/structs/List.h>
#include <stddef.h>

typedef struct GameConfig GameConfig;

typedef enum AssetPathType AssetPathType;
typedef enum AssetPathFlags AssetPathFlags;

typedef struct AssetPath AssetPath;

enum AssetPathType
{
	RELATIVE_TO_EXECUTABLE_DIRECTORY,
	ABSOLUTE_PATH,
};

enum AssetPathFlags
{
	/// Allow loading code assets from this asset path. This should be used with caution.
	ASSET_PATH_ALLOW_CODE_EXECUTION = 1 << 0,
	/// This path was added at runtime and is not part of game.gkvl
	ASSET_PATH_RUNTIME_LOADED = 1 << 1,
};

struct AssetPath
{
	AssetPathType type;
	AssetPathFlags flags;
	char *path;
};

struct GameConfig
{
	/// The title of the game, used for the window title
	const char *gameTitle;
	/// The copyright of the game
	const char *gameCopyright;

	/// The Discord developer app ID, used for rich presence
	size_t discordAppId;

	// Asset search paths
	List assetPaths;
};

/// The loaded game config
extern GameConfig gameConfig;

/**
 * Load the game config asset
 */
void LoadGameConfig(const char *game);

void DestroyGameConfig();

#endif //GAME_GAMECONFIGLOADER_H
