//
// Created by droc101 on 4/26/2024.
//

/**
 * @file AssetReader.h
 * Manages asset file loading and cachine
 */

#ifndef GAME_ASSETREADER_H
#define GAME_ASSETREADER_H

#include <engine/structs/Asset.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/// Asset container format version
#define ASSET_FORMAT_VERSION 2

/**
 * Prints an error and returns NULL if there are not enough bytes remaining to read
 * @param expected The number of bytes expected to read
 * @param bytesRemaining A variable containing the number of bytes remaining in the buffer. This macro will modify it.
 * @warning This may leak memory due to early returns
 */
#define EXPECT_BYTES(expected, bytesRemaining) \
	{ \
		if ((bytesRemaining) < (expected)) \
		{ \
			LogError("Not enough bytes remaining to read %zu bytes\n", (expected)); \
			return NULL; \
		} \
		(bytesRemaining) -= (expected); \
	}

/**
 * Prints an error and returns false if there are not enough bytes remaining to read
 * @param expected The number of bytes expected to read
 * @param bytesRemaining A variable containing the number of bytes remaining in the buffer. This macro will modify it.
 * @warning This may leak memory due to early returns
 */
#define EXPECT_BYTES_BOOL(expected, bytesRemaining) \
	{ \
		if ((bytesRemaining) < (expected)) \
		{ \
			LogError("Not enough bytes remaining to read %zu bytes\n", (expected)); \
			return false; \
		} \
		(bytesRemaining) -= (expected); \
	}

/**
 * Expects bytesRemaining to be zero, and prints a warning if it is not
 */
#define EXPECT_EOF_BYTES(bytesRemaining) \
	{ \
		if ((bytesRemaining) > 0) \
		{ \
			LogWarning("%s: Expected end of buffer, but %zu bytes remain!\n", __PRETTY_FUNCTION__, bytesRemaining); \
		} \
	}

/**
 * Initialize the asset cache
 * @private
 */
void AssetCacheInit();

/**
 * Invalidate the asset cache
 * @warning If anything still has a pointer to an asset, it will become invalid!
 * @private
 */
void DestroyAssetCache();

/**
 * Get a list of all assets of a certain type that are in a folder, across all asset paths
 * @param folder The folder to recursively enumerate the assets of
 * @param output Where to store the list of assets
 * @param extension Asset file extension to search for
 */
void EnumerateAssetsInFolder(const char *folder, List *output, const char *extension);

/**
 * Create an asset directly from a file handle. This does NOT cache the asset, as it has no associated path.
 * @param file The file to create the asset from
 * @return A pointer to an Asset, or NULL on failure
 */
Asset *LoadAssetFromFile(FILE *file);

/**
 * Decompress an asset and cache it
 * @param relPath The asset to decompress
 * @param cache Whether the asset should be cached
 * @param isCodeAsset Whether the asset is considered code, when true it will not search asset paths without @c ASSET_PATH_ALLOW_CODE_EXECUTION set
 * @return Decompressed asset, including header
 * @warning If the asset is not cached, you will have to pass it to @c FreeAsset. Otherwise,
 * it is kept around until program exit and automatically freed.
 */
Asset *LoadAsset(const char *relPath, bool cache, bool isCodeAsset);

/**
 * Remove an asset from the cache
 * @param relPath The asset to decompress
 * @note Any pointers to this asset will become invalid.
 */
void RemoveAssetFromCache(const char *relPath);

/**
 * Hot reload all cached assets. Do not call while a map is loaded.
 */
void HotReloadAssets();

/// @private
void DebugEntryAssetReader();

/// Create a fully qualified texture path
#define TEXTURE(textureName) ("texture/" textureName ".gtex")
/// Create a fully qualified sound path
#define SOUND(soundName) ("sound/" soundName ".gsnd")
/// Create a fully qualified map path
#define MAP(mapName) ("map/" mapName ".gmap")
/// Create a fully qualified shader path
#define SHADER(shaderName) ("shader/" shaderName ".gshd")
/// Create a fully qualified model path
#define MODEL(modelName) ("model/" modelName ".gmdl")
/// Create a fully qualified font path
#define FONT(fontName) ("font/" fontName ".gfon")
/// Create a fully qualified material path
#define MAP_MATERIAL(materialName) ("material/" materialName ".gmtl")

#endif //GAME_ASSETREADER_H
