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
#include "../Graphics/RenderingHelpers.h"
#include "DataReader.h"
#include "Error.h"
#include "Logging.h"

List assetCacheNames;
List assetCacheData;
uint textureId;
size_t modelId;
size_t lodId;
size_t materialId;
Image *images[MAX_TEXTURES];
ModelDefinition *models[MAX_MODELS];

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

void FreeModel(ModelDefinition *model)
{
	if (model == NULL)
	{
		return;
	}
	for (int i = 0; i < model->skinCount; i++)
	{
		free(model->skins[i]);
	}

	for (int i = 0; i < model->lodCount; i++)
	{
		ModelLod *lod = model->lods[i];
		free(lod->vertexData);
		for (int j = 0; j < model->materialCount; j++)
		{
			free(lod->indexData[j]);
		}
		free(lod->indexData);
		free(lod->indexCount);
		free(lod);
	}

	free(model->name);
	free(model->skins);
	free(model->lods);
	free(model);

	model = NULL;
}

void AssetCacheInit()
{
	ListCreate(&assetCacheNames);
	ListCreate(&assetCacheData);
}

void DestroyAssetCache()
{
	for (int i = 0; i < assetCacheData.length; i++)
	{
		Asset *asset = ListGet(assetCacheData, i);
		free(asset->data);
		free(asset);
	}
	free(assetCacheData.data);

	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		if (images[i] != NULL)
		{
			free(images[i]->name);
			free(images[i]);
		}
	}

	for (int i = 0; i < MAX_MODELS; i++)
	{
		FreeModel(models[i]);
	}

	ListAndContentsFree(&assetCacheNames, false);
}

Asset *DecompressAsset(const char *relPath)
{
	// see if relPath is already in the cache
	for (int i = 0; i < assetCacheNames.length; i++)
	{
		if (strncmp(ListGet(assetCacheNames, i), relPath, 80) == 0)
		{
			return ListGet(assetCacheData, i);
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
	const uint compressedSize = ReadUint(asset, &offset);
	const uint decompressedSize = ReadUint(asset, &offset);
	offset += sizeof(uint); // skip asset ID as it is no longer used
	const uint assetType = ReadUint(asset, &offset);

	assetStruct->compressedSize = compressedSize;
	assetStruct->size = decompressedSize;
	assetStruct->type = assetType;

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

	// Add the asset to the cache
	const size_t pathLength = strlen(relPath) + 1;
	char *data = malloc(pathLength);
	CheckAlloc(data);
	strncpy(data, relPath, pathLength);
	ListAdd(&assetCacheNames, data);
	ListAdd(&assetCacheData, assetStruct);

	free(asset);

	return assetStruct;
}

void RemoveAssetFromCache(const char *relPath)
{
	int index = -1;
	for (int i = 0; i < assetCacheNames.length; i++)
	{
		if (strncmp(ListGet(assetCacheNames, i), relPath, 80) == 0)
		{
			index = i;
			break;
		}
	}
	if (index != -1)
	{
		free(ListGet(assetCacheNames, index));
		ListRemoveAt(&assetCacheNames, index);
		free(ListGet(assetCacheData, index));
		ListRemoveAt(&assetCacheData, index);
	} else
	{
		LogWarning("Was told to remove \"%s\" from the asset cache, but it was not present.\n", relPath);
	}
}

void GenFallbackImage(Image *src)
{
	src->width = 64;
	src->height = 64;
	src->pixelDataSize = 64 * 64 * 4;
	src->pixelData = malloc(src->pixelDataSize);
	CheckAlloc(src->pixelData);

	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 64; y++)
		{
			if ((x < 32) ^ (y < 32))
			{
				src->pixelData[(x + y * 64) * 4] = 0;
				src->pixelData[(x + y * 64) * 4 + 1] = 0;
				src->pixelData[(x + y * 64) * 4 + 2] = 0;
				src->pixelData[(x + y * 64) * 4 + 3] = 255;
			} else
			{
				src->pixelData[(x + y * 64) * 4] = 255;
				src->pixelData[(x + y * 64) * 4 + 1] = 0;
				src->pixelData[(x + y * 64) * 4 + 2] = 255;
				src->pixelData[(x + y * 64) * 4 + 3] = 255;
			}
		}
	}
}

Image *LoadImage(const char *asset)
{
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		Image *img = images[i];
		if (img == NULL)
		{
			break;
		}
		if (strncmp(asset, img->name, 80) == 0)
		{
			return img;
		}
	}

	if (textureId >= MAX_TEXTURES)
	{
		Error("Texture ID heap exhausted. Please increase MAX_TEXTURES");
	}

	Image *img = malloc(sizeof(Image));
	CheckAlloc(img);

	const Asset *textureAsset = DecompressAsset(asset);
	if (textureAsset == NULL || textureAsset->type != ASSET_TYPE_TEXTURE)
	{
		GenFallbackImage(img);
	} else
	{
		img->pixelDataSize = ReadUintA(textureAsset->data, IMAGE_SIZE_OFFSET);
		img->width = ReadUintA(textureAsset->data, IMAGE_WIDTH_OFFSET);
		img->height = ReadUintA(textureAsset->data, IMAGE_HEIGHT_OFFSET);
		img->pixelData = textureAsset->data + sizeof(uint) * 4;
	}

	img->id = textureId;

	const size_t nameLength = strlen(asset) + 1;
	img->name = malloc(nameLength);
	CheckAlloc(img->name);
	strncpy(img->name, asset, nameLength);

	images[textureId] = img;

	textureId++;

	if (textureId >= MAX_TEXTURES - 10)
	{
		LogWarning("Texture ID heap is nearly exhausted! Only %lu slots remain.\n", MAX_TEXTURES - textureId);
	}

	RemoveAssetFromCache(asset);

	return img;
}

ModelDefinition *LoadModel(const char *asset)
{
	for (int i = 0; i < MAX_MODELS; i++)
	{
		ModelDefinition *model = models[i];
		if (model == NULL)
		{
			break;
		}
		if (strncmp(asset, model->name, 80) == 0)
		{
			return model;
		}
	}

	if (modelId >= MAX_MODELS)
	{
		Error("Model ID heap exhausted. Please increase MAX_MODELS");
	}

	const Asset *assetData = DecompressAsset(asset);
	if (assetData == NULL)
	{
		LogError("Failed to load model from asset, asset was NULL!");
		Error("Failed to load model!");
	}
	ModelDefinition *model = malloc(sizeof(ModelDefinition));
	CheckAlloc(model);

	model->id = modelId;
	models[modelId] = model;
	modelId++;

	const size_t nameLength = strlen(asset) + 1;
	model->name = malloc(nameLength);
	CheckAlloc(model->name);
	strncpy(model->name, asset, nameLength);

	size_t offset = 0;
	model->materialCount = ReadUint(assetData->data, &offset);
	model->skinCount = ReadUint(assetData->data, &offset);
	model->lodCount = ReadUint(assetData->data, &offset);
	model->skins = malloc(sizeof(Material *) * model->skinCount);
	CheckAlloc(model->skins);

	const size_t skinSize = sizeof(Material) * model->materialCount;
	for (int i = 0; i < model->skinCount; i++)
	{
		model->skins[i] = malloc(skinSize);
		CheckAlloc(model->skins[i]);
		Material *skin = model->skins[i];
		for (int j = 0; j < model->materialCount; j++)
		{
			Material *mat = &skin[j];

			mat->id = materialId;
			materialId++;

			ReadString(assetData->data, &offset, mat->texture, 64);
			GetColor(ReadUint(assetData->data, &offset), &mat->color);
			mat->shader = ReadUint(assetData->data, &offset);
		}
	}

	model->lods = malloc(sizeof(ModelLod *) * model->lodCount);
	CheckAlloc(model->lods);
	for (int i = 0; i < model->lodCount; i++)
	{
		model->lods[i] = malloc(sizeof(ModelLod));
		CheckAlloc(model->lods[i]);
		ModelLod *lod = model->lods[i];

		lod->id = lodId;
		lodId++;

		lod->distance = ReadFloat(assetData->data, &offset);
		lod->vertexCount = ReadUint(assetData->data, &offset);

		const size_t vertexDataSize = lod->vertexCount * sizeof(float) * 8;
		lod->vertexData = malloc(vertexDataSize);
		CheckAlloc(lod->vertexData);
		ReadBytes(assetData->data, &offset, vertexDataSize, lod->vertexData);

		lod->totalIndexCount = ReadUint(assetData->data, &offset);
		const size_t indexCountSize = model->materialCount * sizeof(uint);
		lod->indexCount = malloc(indexCountSize);
		CheckAlloc(lod->indexCount);
		ReadBytes(assetData->data, &offset, indexCountSize, lod->indexCount);

		lod->indexData = malloc(sizeof(uint *) * model->materialCount);
		CheckAlloc(lod->indexData);
		for (int j = 0; j < model->materialCount; j++)
		{
			uint *indexData = malloc(lod->indexCount[j] * sizeof(uint));
			CheckAlloc(indexData);
			lod->indexData[j] = indexData;
			ReadBytes(assetData->data, &offset, lod->indexCount[j] * sizeof(uint), indexData);
		}
	}

	if (modelId >= MAX_MODELS - 10)
	{
		LogWarning("Model ID heap is nearly exhausted! Only %lu slots remain.\n", MAX_MODELS - modelId);
	}

	RemoveAssetFromCache(asset);

	return model;
}

inline ModelDefinition *GetModelFromId(const uint id)
{
	if (id >= modelId)
	{
		Error("Invalid model ID!");
	}

	return models[id];
}

Font *LoadFont(const char *asset)
{
	const Asset *assetData = DecompressAsset(asset);
	if (assetData == NULL)
	{
		LogError("Failed to load font from asset, asset was NULL!");
		Error("Failed to load model!");
	}
	if (assetData->size < sizeof(Font) - sizeof(Image *))
	{
		LogError("Failed to load font from asset, size was too small!");
		return NULL;
	}
	Font *font = malloc(sizeof(Font));
	CheckAlloc(font);
	memcpy(font, assetData->data, sizeof(Font) - sizeof(Image *));
	char temp[64];
	strncpy(temp, font->texture, 64);
	snprintf(font->texture, 80, "texture/%s.gtex", temp);
	font->image = LoadImage(font->texture);

	return font;
}
