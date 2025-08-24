//
// Created by droc101 on 4/26/2024.
//

#ifndef GAME_ASSETREADER_H
#define GAME_ASSETREADER_H

#include <stdbool.h>
#include <stddef.h>
#include "../../Structs/Asset.h"

#define ASSET_FORMAT_VERSION 2
#define ASSET_FORMAT_MAGIC 0x454D4147

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
#define LEVEL(assetName) ("level/" assetName ".gmap")
#define SHADER(assetName) ("shader/" assetName ".gshd")
#define MODEL(assetName) ("model/" assetName ".gmdl")
#define FONT(assetName) ("font/" assetName ".gfon")

#endif //GAME_ASSETREADER_H
