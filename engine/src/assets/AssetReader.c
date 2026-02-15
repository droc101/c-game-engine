//
// Created by droc101 on 4/26/2024.
//

#include <assert.h>
#include <dirent.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/structs/Asset.h>
#include <engine/structs/Dict.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
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

#include "engine/assets/GameConfigLoader.h"

DEFINE_DICT(AssetCache, const char *, M_CSTR_OPLIST, Asset, ASSET_OPLIST);

AssetCache assetCache;

FILE *OpenAssetFile(const char *relPath, const bool isCodeAsset)
{
	const size_t maxPathLength = 300;
	// const size_t pathLen = assetPathLen + strlen("/") + strlen(relPath) + 1;
	char *path = calloc(maxPathLength, sizeof(char));
	CheckAlloc(path);
	for (size_t i = 0; i < gameConfig.assetPaths.length; i++)
	{
		const AssetPath *assetPath = ListGetPointer(gameConfig.assetPaths, i);
		if (isCodeAsset && !(assetPath->flags & ASSET_PATH_ALLOW_CODE_EXECUTION))
		{
			continue;
		}
		const size_t pathLen = strlen(assetPath->path) + 1 + strlen(relPath) + 1;
		if (pathLen >= maxPathLength)
		{
			LogError("Path is too long: %s\n", relPath);
			continue;
		}
		if (snprintf(path, maxPathLength, "%s/%s", assetPath->path, relPath) > 300)
		{
			LogError("Asset path too long!\n");
			continue;
		}

		FILE *file = fopen(path, "rb");
		if (file == NULL)
		{
			continue;
		}

		free(path);

		return file;
	}
	LogError("Failed to open asset file: %s\n", relPath);

	free(path);
	return NULL;
}

void EnumerateAssetsInFolder(const char *folder, List *output)
{
	// TODO don't add duplicates (assets overwritten in another asset path)
	ListClear(*output);
	for (size_t i = 0; i < gameConfig.assetPaths.length; i++)
	{
		const AssetPath *assetPath = ListGetPointer(gameConfig.assetPaths, i);
		char levelDataPath[300];
		sprintf(levelDataPath, "%s/%s/", assetPath->path, folder);

		// Get the name of all gmap files in the level directory
		DIR *dir = opendir(levelDataPath);
		if (dir == NULL)
		{
			LogError("Failed to open level directory: %s\n", levelDataPath);
			return;
		}

		const struct dirent *ent = readdir(dir);
		while (ent != NULL)
		{
			if (strstr(ent->d_name, ".gmap") != NULL)
			{
				char *levelName = malloc(strlen(ent->d_name) + 1);
				CheckAlloc(levelName);
				strcpy(levelName, ent->d_name);
				// Remove the .gmap extension
				levelName[strlen(levelName) - 5] = '\0';
				ListAdd(*output, levelName);
			}
			ent = readdir(dir);
		}
		closedir(dir);
	}
}

void AssetCacheInit()
{
	LogDebug("Initializing asset cache...\n");
	AssetCache_init(assetCache);
	InitModelLoader();
}

void DestroyAssetCache()
{
	LogDebug("Cleaning up asset cache...\n");
	AssetCache_clear(assetCache);
	DestroyTextureLoader();
	DestroyModelLoader();
}

Asset *CreateAssetFromFile(FILE *file)
{
	fseek(file, 0, SEEK_END);
	const size_t fileSize = ftell(file);

	uint8_t *assetData = malloc(fileSize);
	CheckAlloc(assetData);
	fseek(file, 0, SEEK_SET);
	const size_t bytesRead = fread(assetData, 1, fileSize, file);
	if (bytesRead != fileSize)
	{
		free(assetData);
		fclose(file);
		LogError("Failed to read asset file\n");
		return NULL;
	}

	fclose(file);

	size_t offset = 0;
	// Read the first 4 bytes of the asset to get the size of the compressed data
	const uint32_t magic = ReadUint(assetData, &offset);
	if (magic != ASSET_FORMAT_MAGIC)
	{
		free(assetData);
		LogError("Failed to read an asset because the magic was incorrect.\n");
		return NULL;
	}
	const uint8_t assetVersion = ReadByte(assetData, &offset);
	if (assetVersion != ASSET_FORMAT_VERSION)
	{
		free(assetData);
		LogError("Failed to read an asset because the version was incorrect.\n");
		return NULL;
	}
	const uint8_t assetType = ReadByte(assetData, &offset);
	const uint8_t typeVersion = ReadByte(assetData, &offset);
	const size_t decompressedSize = ReadSizeT(assetData, &offset);
	const size_t compressedSize = ReadSizeT(assetData, &offset);

	// Allocate memory for the decompressed data
	uint8_t *decompressedData = malloc(decompressedSize);
	CheckAlloc(decompressedData);

	z_stream stream = {0};

	// Initialize the zlib stream
	stream.next_in = assetData + offset; // skip header
	stream.avail_in = compressedSize;
	stream.next_out = decompressedData;
	stream.avail_out = decompressedSize;

	// Initialize the zlib stream
	if (inflateInit2(&stream, MAX_WBITS | 16) != Z_OK)
	{
		free(decompressedData);
		free(assetData);
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
			free(assetData);
			LogError("Failed to decompress zlib stream: %s\n", stream.msg);
			return NULL;
		}
		inflateReturnValue = inflate(&stream, Z_NO_FLUSH);
	}

	// Clean up the zlib stream
	if (inflateEnd(&stream) != Z_OK)
	{
		free(decompressedData);
		free(assetData);
		LogError("Failed to end zlib stream: %s\n", stream.msg);
		return NULL;
	}

	free(assetData);

	Asset *asset = malloc(sizeof(Asset));
	CheckAlloc(asset);

	asset->compressedSize = compressedSize;
	asset->size = decompressedSize;
	asset->type = assetType;
	asset->typeVersion = typeVersion;
	asset->data = decompressedData;

	return asset;
}

Asset *DecompressAsset(const char *relPath, const bool cache, const bool isCodeAsset)
{
	Asset *asset = NULL;
	if (cache)
	{
		asset = AssetCache_get(assetCache, relPath);
		if (asset != NULL)
		{
			return asset;
		}
	}

	FILE *file = OpenAssetFile(relPath, isCodeAsset);
	if (file == NULL)
	{
		LogError("Failed to open asset file: %s\n", relPath);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	const size_t fileSize = ftell(file);

	uint8_t *assetData = malloc(fileSize);
	CheckAlloc(assetData);
	fseek(file, 0, SEEK_SET);
	const size_t bytesRead = fread(assetData, 1, fileSize, file);
	if (bytesRead != fileSize)
	{
		free(assetData);
		fclose(file);
		LogError("Failed to read asset file: %s\n", relPath);
		return NULL;
	}

	fclose(file);

	if (fileSize < ASSET_HEADER_SIZE)
	{
		LogError("Trying to read asset file of size %zu, which is too small. Refusing to read this asset.\n", fileSize);
		free(assetData);
		return NULL;
	}

	size_t offset = 0;
	// Read the first 4 bytes of the asset to get the size of the compressed data
	const uint32_t magic = ReadUint(assetData, &offset);
	if (magic != ASSET_FORMAT_MAGIC)
	{
		free(assetData);
		LogError("Failed to read an asset because the magic was incorrect.\n");
		return NULL;
	}
	const uint8_t assetVersion = ReadByte(assetData, &offset);
	if (assetVersion != ASSET_FORMAT_VERSION)
	{
		free(assetData);
		LogError("Failed to read an asset because the version was incorrect.\n");
		return NULL;
	}
	const uint8_t assetType = ReadByte(assetData, &offset);
	const uint8_t typeVersion = ReadByte(assetData, &offset);
	const size_t decompressedSize = ReadSizeT(assetData, &offset);
	const size_t compressedSize = ReadSizeT(assetData, &offset);

	if (fileSize - ASSET_HEADER_SIZE != compressedSize)
	{
		LogError("Asset misreported compressedSize as %zu, while the file has %zu bytes remaining. Refusing to read this asset.\n", fileSize - ASSET_HEADER_SIZE, compressedSize);
		free(assetData);
		return NULL;
	}

	// Allocate memory for the decompressed data
	uint8_t *decompressedData = malloc(decompressedSize);
	CheckAlloc(decompressedData);

	z_stream stream = {0};

	// Initialize the zlib stream
	stream.next_in = assetData + ASSET_HEADER_SIZE;
	stream.avail_in = compressedSize;
	stream.next_out = decompressedData;
	stream.avail_out = decompressedSize;

	// Initialize the zlib stream
	if (inflateInit2(&stream, MAX_WBITS | 16) != Z_OK)
	{
		free(decompressedData);
		free(assetData);
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
			free(assetData);
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
		free(assetData);
		LogError("Failed to end zlib stream: %s\n", stream.msg);
		return NULL;
	}

	free(assetData);

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
