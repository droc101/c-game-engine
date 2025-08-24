//
// Created by NBT22 on 8/24/25.
//

#ifndef GAME_ASSET_H
#define GAME_ASSET_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum AssetType AssetType;
typedef struct Asset Asset;

enum AssetType
{
	ASSET_TYPE_TEXTURE = 0,
	ASSET_TYPE_WAV = 1,
	ASSET_TYPE_LEVEL = 2,
	ASSET_TYPE_SHADER = 3,
	ASSET_TYPE_MODEL = 4,
	ASSET_TYPE_FONT = 5
};

struct Asset
{
	/// The compressed size of the asset, excluding the header
	size_t compressedSize;
	/// The decompressed size of the asset
	size_t size;
	/// The type of the asset
	AssetType type;
	/// The version of the type
	uint8_t typeVersion;
	/// The data of the asset
	uint8_t *data;
};

#define ASSET_ZERO(asset) memset(&(asset), 0, sizeof(asset));
#define ASSET_COPY(asset, value) \
	free((asset).data); \
	memcpy(&(asset), &(value), sizeof(asset));
#define ASSET_FREE(asset) \
	free((asset).data); \
	ASSET_ZERO(asset);
#define ASSET_INIT_WITH(obj, compressedSizeValue, sizeValue, typeValue, typeVersionValue, dataValue) \
	(obj).compressedSize = (compressedSizeValue); \
	(obj).size = (sizeValue); \
	(obj).type = (typeValue); \
	(obj).typeVersion = (typeVersionValue); \
	(obj).data = (dataValue);

#define ASSET_OPLIST \
	(INIT(ASSET_ZERO), \
	 INIT_SET(ASSET_COPY), \
	 INIT_WITH(ASSET_INIT_WITH), \
	 SET(ASSET_COPY), \
	 CLEAR(ASSET_FREE), \
	 EMPLACE_TYPE((size_t, size_t, AssetType, uint8_t, uint8_t *)))

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
