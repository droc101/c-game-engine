//
// Created by droc101 on 4/26/2024.
//

#ifndef GAME_ASSETREADER_H
#define GAME_ASSETREADER_H

#include <engine/structs/Asset.h>
#include <stdbool.h>
#include <stddef.h>

#define ASSET_FORMAT_VERSION 2
#define ASSET_FORMAT_MAGIC 0x454D4147
#define ASSET_HEADER_SIZE (sizeof(uint32_t) + (sizeof(uint8_t) * 3) + (sizeof(size_t) * 2))

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
 * Initialize the asset cache
 */
void AssetCacheInit();

/**
 * Invalidate the asset cache
 * @warning If anything still has a pointer to an asset, it will become invalid!
 */
void DestroyAssetCache();

/**
 * Decompress an asset and cache it
 * @param relPath The asset to decompress
 * @param cache Whether the asset should be cached
 * @return Decompressed asset, including header
 * @warning If the asset is not cached, you will have to pass it to @c FreeAsset. Otherwise,
 * it is kept around until program exit and automatically freed.
 */
Asset *DecompressAsset(const char *relPath, bool cache);

/**
 * Remove an asset from the cache
 * @param relPath The asset to decompress
 * @note Any pointers to this asset will become invalid.
 */
void RemoveAssetFromCache(const char *relPath);

#define TEXTURE(assetName) ("texture/" assetName ".gtex")
#define SOUND(assetName) ("sound/" assetName ".gsnd")
#define MAP(assetName) ("map/" assetName ".gmap")
#define SHADER(assetName) ("shader/" assetName ".gshd")
#define MODEL(assetName) ("model/" assetName ".gmdl")
#define FONT(assetName) ("font/" assetName ".gfon")
#define MAP_MATERIAL(assetName) ("material/" assetName ".gmtl")

#endif //GAME_ASSETREADER_H
