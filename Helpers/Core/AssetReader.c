//
// Created by droc101 on 4/26/2024.
//

#include "AssetReader.h"
#include <assert.h>
#include <errno.h>
#include <m-core.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zconf.h>
#include <zlib.h>
#include "../../Structs/Asset.h"
#include "../../Structs/GlobalState.h"
#include "AssetLoaders/ModelLoader.h"
#include "AssetLoaders/TextureLoader.h"
#include "DataReader.h"
#include "Dict.h"
#include "Error.h"
#include "Logging.h"

DICT_DEF(AssetCache, const char *, M_CSTR_OPLIST, Asset, ASSET_OPLIST);

AssetCache assetCache;

FILE *OpenAssetFile(const char *relPath)
{
	const size_t maxPathLength = 300;
	char *path = calloc(maxPathLength, sizeof(char));
	CheckAlloc(path);

	const size_t pathLen = strlen(GetState()->executableFolder) + strlen("assets/") + strlen(relPath) + 1;
	if (pathLen >= maxPathLength)
	{
		LogError("Path is too long: %s\n", relPath);
		free(path);
		return NULL;
	}
	if (snprintf(path, maxPathLength, "%sassets/%s", GetState()->executableFolder, relPath) > 300)
	{
		LogError("Asset path too long!\n");
		free(path);
		return NULL;
	}

	FILE *file = fopen(path, "rb");
	if (file == NULL)
	{
		LogError("Failed to open asset file: %s with errno %s\n", path, strerror(errno));
		free(path);
		return NULL;
	}

	free(path);

	return file;
}

void AssetCacheInit()
{
	AssetCache_init(assetCache);
	InitModelLoader();
}

void DestroyAssetCache()
{
	AssetCache_clear(assetCache);

	DestroyTextureLoader();
	DestroyModelLoader();
}

Asset *DecompressAsset(const char *relPath, const bool cache)
{
	Asset *asset = AssetCache_get(assetCache, relPath);
	if (asset != NULL)
	{
		return asset;
	}

	FILE *file = OpenAssetFile(relPath);
	if (file == NULL)
	{
		LogError("Failed to open asset file: %s\n", relPath);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	const size_t fileSize = ftell(file);

	uint8_t *assetPath = malloc(fileSize);
	CheckAlloc(assetPath);
	fseek(file, 0, SEEK_SET);
	const size_t bytesRead = fread(assetPath, 1, fileSize, file);
	if (bytesRead != fileSize)
	{
		free(assetPath);
		fclose(file);
		LogError("Failed to read asset file: %s\n", relPath);
		return NULL;
	}

	fclose(file);

	size_t offset = 0;
	// Read the first 4 bytes of the asset to get the size of the compressed data
	const uint32_t magic = ReadUint(assetPath, &offset);
	if (magic != ASSET_FORMAT_MAGIC)
	{
		free(assetPath);
		LogError("Failed to read an asset because the magic was incorrect.\n");
		return NULL;
	}
	const uint8_t assetVersion = ReadByte(assetPath, &offset);
	if (assetVersion != ASSET_FORMAT_VERSION)
	{
		free(assetPath);
		LogError("Failed to read an asset because the version was incorrect.\n");
		return NULL;
	}
	const uint8_t assetType = ReadByte(assetPath, &offset);
	const uint8_t typeVersion = ReadByte(assetPath, &offset);
	const size_t decompressedSize = ReadSizeT(assetPath, &offset);
	const size_t compressedSize = ReadSizeT(assetPath, &offset);

	// Allocate memory for the decompressed data
	uint8_t *decompressedData = malloc(decompressedSize);
	CheckAlloc(decompressedData);

	z_stream stream = {0};

	// Initialize the zlib stream
	stream.next_in = assetPath + offset; // skip header
	stream.avail_in = compressedSize;
	stream.next_out = decompressedData;
	stream.avail_out = decompressedSize;

	// Initialize the zlib stream
	if (inflateInit2(&stream, MAX_WBITS | 16) != Z_OK)
	{
		free(decompressedData);
		free(assetPath);
		free(asset);
		LogError("Failed to initialize zlib stream: %s\n", stream.msg);
		return NULL;
	}

	// Decompress the data
	int inflateReturnValue = inflate(&stream, Z_NO_FLUSH);
	while (inflateReturnValue != Z_STREAM_END)
	{
		if (inflateReturnValue != Z_OK)
		{
			free(decompressedData);
			free(assetPath);
			free(asset);
			LogError("Failed to decompress zlib stream: %s\n", stream.msg);
			return NULL;
		}
		inflateReturnValue = inflate(&stream, Z_NO_FLUSH);
	}

	// Clean up the zlib stream
	if (inflateEnd(&stream) != Z_OK)
	{
		free(decompressedData);
		free(assetPath);
		LogError("Failed to end zlib stream: %s\n", stream.msg);
		return NULL;
	}

	free(assetPath);

	if (cache)
	{
		asset = AssetCache_safe_get(assetCache, relPath);
	} else
	{
		asset = malloc(sizeof(Asset));
		CheckAlloc(asset);
	}

	asset->compressedSize = compressedSize;
	asset->size = decompressedSize;
	asset->type = assetType;
	asset->typeVersion = typeVersion;
	asset->data = decompressedData;

	return asset;
}

void RemoveAssetFromCache(const char *relPath)
{
	AssetCache_erase(assetCache, relPath);
}
