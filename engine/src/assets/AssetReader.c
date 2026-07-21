//
// Created by droc101 on 4/26/2024.
//

#include <assert.h>
#include <dirent.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/assets/MapMaterialLoader.h>
#include <engine/assets/ModelLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/debug/DPrint.h>
#include <engine/graphics/Font.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/Asset.h>
#include <engine/structs/Color.h>
#include <engine/structs/Dict.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/SoundSystem.h>
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

DEFINE_DICT(AssetCache, const char *, M_CSTR_OPLIST, Asset, ASSET_OPLIST);

static AssetCache assetCache;

static FILE *OpenAssetFile(const char *relPath, const bool isCodeAsset)
{
	if (strlen(relPath) == 0)
	{
		LogError("Asset name must not be empty!\n");
		return NULL;
	}
	const size_t maxPathLength = 300;
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
		if (snprintf(path, maxPathLength, "%s/%s", assetPath->path, relPath) > (int64_t)maxPathLength)
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

void EnumerateAssetsInFolder(const char *folder, List *output, const char *extension)
{
	// TODO this should do subfolders
	ListFreeOnlyContents(*output);
	ListClear(*output);
	for (size_t i = 0; i < gameConfig.assetPaths.length; i++)
	{
		const AssetPath *assetPath = ListGetPointer(gameConfig.assetPaths, i);
		char levelDataPath[300];
		if (snprintf(levelDataPath, 300, "%s/%s/", assetPath->path, folder) > 300)
		{
			LogError("Asset directory path is too long: %s/%s\n", assetPath->path, folder);
			continue;
		}

		DIR *dir = opendir(levelDataPath);
		if (dir == NULL)
		{
			if (errno != ENOENT)
			{
				LogError("Failed to open directory: %s\nError: %s\n", levelDataPath, strerror(errno));
			}
			continue;
		}

		const struct dirent *ent = readdir(dir);
		while (ent != NULL)
		{
			if (strstr(ent->d_name, extension) != NULL)
			{
				char *levelName = malloc(strlen(ent->d_name) + 1);
				CheckAlloc(levelName);
				strcpy(levelName, ent->d_name);
				// Remove the .gmap extension
				levelName[strlen(levelName) - strlen(extension)] = '\0';

				bool found = false;
				for (size_t j = 0; j < output->length; j++)
				{
					const char *existing = ListGetPointer(*output, j);
					if (strcmp(levelName, existing) == 0)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					ListAdd(*output, levelName);
				}
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
	DestroyMapMaterialLoader();
}

static bool DecompressAsset(FILE *file, Asset *dest)
{
	CheckAlloc(dest);
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
		return false;
	}

	fclose(file);

	DataReader *reader = CreateDataReader(assetData, fileSize, 0);

	const uint32_t magic = ReadUint32(reader);
	if (magic != ASSET_FORMAT_MAGIC)
	{
		free(assetData);
		DestroyDataReader(reader);
		LogError("Failed to read an asset because the magic was incorrect.\n");
		return false;
	}
	const uint8_t assetVersion = ReadUint8(reader);
	if (assetVersion != ASSET_FORMAT_VERSION)
	{
		free(assetData);
		DestroyDataReader(reader);
		LogError("Failed to read an asset because the version was incorrect.\n");
		return false;
	}
	const uint8_t assetType = ReadUint8(reader);
	const uint8_t typeVersion = ReadUint8(reader);
	const size_t decompressedSize = ReadSizeT(reader);
	const size_t compressedSize = ReadSizeT(reader);

	if (fileSize - ASSET_HEADER_SIZE != compressedSize)
	{
		LogError("Asset misreported compressedSize as %zu, while the file has %zu bytes remaining. Refusing to read "
				 "this asset.\n",
				 compressedSize,
				 fileSize - ASSET_HEADER_SIZE);
		free(assetData);
		DestroyDataReader(reader);
		return false;
	}

	// Allocate memory for the decompressed data
	uint8_t *decompressedData = malloc(decompressedSize);
	CheckAlloc(decompressedData);

	z_stream stream = {0};

	// Initialize the zlib stream
	stream.next_in = assetData + DataReaderGetOffset(reader); // skip header
	stream.avail_in = compressedSize;
	stream.next_out = decompressedData;
	stream.avail_out = decompressedSize;

	DestroyDataReader(reader);

	// Initialize the zlib stream
	if (inflateInit2(&stream, MAX_WBITS | 16) != Z_OK)
	{
		free(decompressedData);
		free(assetData);
		LogError("Failed to initialize zlib stream: %s\n", stream.msg);
		return false;
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
			return false;
		}
		inflateReturnValue = inflate(&stream, Z_NO_FLUSH);
	}

	// Clean up the zlib stream
	if (inflateEnd(&stream) != Z_OK)
	{
		free(decompressedData);
		free(assetData);
		LogError("Failed to end zlib stream: %s\n", stream.msg);
		return false;
	}

	free(assetData);
	dest->size = decompressedSize;
	dest->type = assetType;
	dest->typeVersion = typeVersion;
	dest->data = decompressedData;

	return true;
}

Asset *LoadAssetFromFile(FILE *file)
{
	Asset *asset = malloc(sizeof(Asset));
	CheckAlloc(asset);
	if (!DecompressAsset(file, asset))
	{
		free(asset);
		return NULL;
	}
	return asset;
}

Asset *LoadAsset(const char *relPath, const bool cache, const bool isCodeAsset)
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

	if (cache)
	{
		asset = AssetCache_safe_get(assetCache, relPath);
	} else
	{
		asset = malloc(sizeof(Asset));
		CheckAlloc(asset);
	}

	if (!DecompressAsset(file, asset))
	{
		free(asset);
		return NULL;
	}
	return asset;
}

void RemoveAssetFromCache(const char *relPath)
{
	AssetCache_erase(assetCache, relPath);
}

void HotReloadAssets()
{
	assert(GetState()->map == NULL);
	StopAllSounds();
	// TODO: This causes a bug where the image IDs will be reset at whatever point within the frame this function is
	//  called, which could cause the image be associated with the wrong GPU texture. This is not simple to fix though,
	//  since reloading the fonts requires that the image cache already be reset, otherwise the font will reference an
	//  image that has been freed. This could be fixed by moving this function into RenderingHelpers.c and only allowing
	//  it to be called by FrameStart. Doing so would also allow this function to directly clear the textures and models
	//  instead of queuing it.
	DestroyCommonFonts();
	DestroyAssetCache();

	AssetCacheInit();
	InitCommonFonts();

	rendererQueuedActions |= QUEUED_ACTION_CLEAR_ALL_TEXTURES | QUEUED_ACTION_CLEAR_ALL_MODELS;
}

void DPrintAssetReader()
{
	DPrintF("Primary Asset Cache: %zu asset(s)", COLOR_WHITE, AssetCache_size(assetCache));
}
