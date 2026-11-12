//
// Created by droc101 on 10/4/25.
//

/**
 * @file GameConfigLoader.h
 * Game configuration (game.gkvl) asset loader and manager
 */

#ifndef GAME_GAMECONFIGLOADER_H
#define GAME_GAMECONFIGLOADER_H

#include <engine/structs/List.h>
#include <stddef.h>

typedef struct GameConfig GameConfig;
typedef struct AssetPath AssetPath;

/// How to interpret an asset path
typedef enum AssetPathType
{
	RELATIVE_TO_EXECUTABLE_DIRECTORY,
	/// if game config dir is "/path/to/game.cfg" this would be relative to "/path"
	RELATIVE_TO_GAME_CONFIG_PARENT_DIRECTORY,
	ABSOLUTE_PATH,
} AssetPathType;

/// Miscellaneous information and behavior options for asset paths
typedef enum AssetPathFlags
{
	/// Allow loading code assets from this asset path. This should be used with caution.
	ASSET_PATH_ALLOW_CODE_EXECUTION = 1 << 0,
	/// This path was added at runtime and is not part of game.gkvl
	ASSET_PATH_RUNTIME_LOADED = 1 << 1,
} AssetPathFlags;

/// A location the engine can search for assets
struct AssetPath
{
	/// The type of this asset path
	AssetPathType type;
	/// The flags of this asset path
	AssetPathFlags flags;
	/// The path of this asset path
	char *path;
};

/// General game configuration information
struct GameConfig
{
	/// The title of the game, used for the window title
	const char *gameTitle;
	/// The copyright of the game
	const char *gameCopyright;

	/// The Discord developer app ID, used for rich presence
	size_t discordAppId;

	/// Asset search paths
	List assetPaths;

	/// The map to load for the menu background
	const char *backgroundMap;
};

/// The loaded game config
extern GameConfig gameConfig;

/**
 * Load the game config asset
 * @private
 */
void LoadGameConfig(const char *game);

/**
 * Destroy the game config asset
 * @private
 */
void DestroyGameConfig();

#endif //GAME_GAMECONFIGLOADER_H
