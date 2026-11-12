//
// Created by NBT22 on 8/24/25.
//

/**
 * @file Asset.h
 * Asset utility functions
 */

#ifndef GAME_ASSET_H
#define GAME_ASSET_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Asset Asset;

/// Asset content type
typedef enum AssetType
{
	ASSET_TYPE_TEXTURE = 0,
	ASSET_TYPE_WAV = 1,
	ASSET_TYPE_MAP = 2,
	ASSET_TYPE_SHADER = 3,
	ASSET_TYPE_MODEL = 4,
	ASSET_TYPE_FONT = 5,
	ASSET_TYPE_MAP_MATERIAL = 7,
	ASSET_TYPE_KV_LIST = 8,
} AssetType;

/// Raw loaded asset
struct Asset
{
	/// The decompressed size of the asset
	size_t size;
	/// The type of the asset
	AssetType type;
	/// The version of the type
	uint8_t typeVersion;
	/// The data of the asset
	uint8_t *data;
};

#define ASSET_OPLIST_ZERO(asset) memset(&(asset), 0, sizeof(asset));
#define ASSET_OPLIST_COPY(asset, value) \
	free((asset).data); \
	memcpy(&(asset), &(value), sizeof(asset));
#define ASSET_OPLIST_FREE(asset) \
	free((asset).data); \
	ASSET_OPLIST_ZERO(asset);

#define ASSET_OPLIST (INIT(ASSET_OPLIST_ZERO), INIT_SET(ASSET_OPLIST_COPY), SET(ASSET_OPLIST_COPY), CLEAR(ASSET_OPLIST_FREE), TYPE(Asset))

/**
 * Free an asset
 * @param asset The asset to free
 * @warning Only use this on assets that are NOT cached.
 */
static inline void FreeAsset(Asset *asset)
{
	if (asset != NULL)
	{
		free(asset->data);
		free(asset);
	}
}

#endif //GAME_ASSET_H
