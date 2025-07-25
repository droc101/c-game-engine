//
// Created by droc101 on 4/26/2024.
//

#include "AssetReader.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "../../Structs/GlobalState.h"
#include "DataReader.h"
#include "Error.h"
#include "Logging.h"
#include "AssetLoaders/TextureLoader.h"
#include "AssetLoaders/ModelLoader.h"

List assetCacheNames;
List assetCacheData;

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
	ListInit(assetCacheNames, LIST_POINTER);
	ListInit(assetCacheData, LIST_POINTER);
	InitModelLoader();
}

void DestroyAssetCache()
{
	for (int i = 0; i < assetCacheData.length; i++)
	{
		Asset *asset = ListGetPointer(assetCacheData, i);
		free(asset->data);
		free(asset);
	}
	ListFree(assetCacheData);

	ListAndContentsFree(assetCacheNames);

	DestroyTextureLoader();
	DestroyModelLoader();
}

Asset *DecompressAsset(const char *relPath, const bool cache)
{
	// see if relPath is already in the cache
	for (int i = 0; i < assetCacheNames.length; i++)
	{
		if (strncmp(ListGetPointer(assetCacheNames, i), relPath, 80) == 0)
		{
			return ListGetPointer(assetCacheData, i);
		}
	}

	FILE *file = OpenAssetFile(relPath);
	if (file == NULL)
	{
		LogError("Failed to open asset file: %s\n", relPath);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	const size_t fileSize = ftell(file);

	byte *asset = malloc(fileSize);
	CheckAlloc(asset);
	fseek(file, 0, SEEK_SET);
	const size_t bytesRead = fread(asset, 1, fileSize, file);
	if (bytesRead != fileSize)
	{
		free(asset);
		fclose(file);
		LogError("Failed to read asset file: %s\n", relPath);
		return NULL;
	}

	fclose(file);

	Asset *assetStruct = malloc(sizeof(Asset));
	CheckAlloc(assetStruct);

	size_t offset = 0;
	// Read the first 4 bytes of the asset to get the size of the compressed data
	const uint32_t magic = ReadUint(asset, &offset);
	if (magic != ASSET_FORMAT_MAGIC)
	{
		free(asset);
		LogError("Failed to read an asset because the magic was incorrect.\n");
		return NULL;
	}
	const uint8_t assetVersion = ReadByte(asset, &offset);
	if (assetVersion != ASSET_FORMAT_VERSION)
	{
		free(asset);
		LogError("Failed to read an asset because the version was incorrect.\n");
		return NULL;
	}
	const uint8_t assetType = ReadByte(asset, &offset);
	const uint8_t typeVersion = ReadByte(asset, &offset);
	const size_t decompressedSize = ReadSizeT(asset, &offset);
	const size_t compressedSize = ReadSizeT(asset, &offset);

	assetStruct->compressedSize = compressedSize;
	assetStruct->size = decompressedSize;
	assetStruct->type = assetType;
	assetStruct->typeVersion = typeVersion;

	// Allocate memory for the decompressed data
	byte *decompressedData = malloc(decompressedSize);
	CheckAlloc(decompressedData);

	z_stream stream = {0};

	// Initialize the zlib stream
	stream.next_in = asset + offset; // skip header
	stream.avail_in = compressedSize;
	stream.next_out = decompressedData;
	stream.avail_out = decompressedSize;

	// Initialize the zlib stream
	if (inflateInit2(&stream, MAX_WBITS | 16) != Z_OK)
	{
		free(decompressedData);
		free(asset);
		free(assetStruct);
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
			free(asset);
			free(assetStruct);
			LogError("Failed to decompress zlib stream: %s\n", stream.msg);
			return NULL;
		}
		inflateReturnValue = inflate(&stream, Z_NO_FLUSH);
	}

	// Clean up the zlib stream
	if (inflateEnd(&stream) != Z_OK)
	{
		free(decompressedData);
		free(asset);
		free(assetStruct);
		LogError("Failed to end zlib stream: %s\n", stream.msg);
		return NULL;
	}

	assetStruct->data = decompressedData;

	if (cache)
	{
		const size_t pathLength = strlen(relPath) + 1;
		char *data = malloc(pathLength);
		CheckAlloc(data);
		strncpy(data, relPath, pathLength);
		ListAdd(assetCacheNames, data);
		ListAdd(assetCacheData, assetStruct);
	}

	free(asset);

	return assetStruct;
}

void RemoveAssetFromCache(const char *relPath)
{
	int index = -1;
	for (int i = 0; i < assetCacheNames.length; i++)
	{
		if (strncmp(ListGetPointer(assetCacheNames, i), relPath, 80) == 0)
		{
			index = i;
			break;
		}
	}
	if (index != -1)
	{
		const Asset *asset = ListGetPointer(assetCacheData, index);
		free(asset->data);
		free(ListGetPointer(assetCacheNames, index));
		ListRemoveAt(assetCacheNames, index);
		free(ListGetPointer(assetCacheData, index));
		ListRemoveAt(assetCacheData, index);
	} else
	{
		LogWarning("Was told to remove \"%s\" from the asset cache, but it was not present.\n", relPath);
	}
}

void FreeAsset(Asset *asset)
{
	free(asset->data);
	free(asset);
}
