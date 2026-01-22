//
// Created by droc101 on 7/23/25.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/structs/Asset.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <SDL3/SDL_endian.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint32_t textureId;
Image *images[MAX_TEXTURES];

#define MISSING_TEX_SIZE 2
#define MISSING_TEX_COLOR_A 0xFF000000
#define MISSING_TEX_COLOR_B 0xFFFF00FF

void GenFallbackImage(Image *src)
{
	src->width = MISSING_TEX_SIZE;
	src->height = MISSING_TEX_SIZE;
	src->filter = false;
	src->repeat = true;
	src->mipmaps = false;
	const size_t pixelDataSize = MISSING_TEX_SIZE * MISSING_TEX_SIZE * sizeof(uint32_t);
	uint32_t *pixelData = malloc(pixelDataSize);
	CheckAlloc(pixelData);
	src->pixelData = (uint8_t *)pixelData;

	for (int x = 0; x < MISSING_TEX_SIZE; x++)
	{
		for (int y = 0; y < MISSING_TEX_SIZE; y++)
		{
			if ((x < MISSING_TEX_SIZE / 2) ^ (y < MISSING_TEX_SIZE / 2))
			{
				pixelData[(x + y * MISSING_TEX_SIZE)] = MISSING_TEX_COLOR_A;
			} else
			{
				pixelData[(x + y * MISSING_TEX_SIZE)] = MISSING_TEX_COLOR_B;
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
		if (strcmp(asset, img->name) == 0)
		{
			return img;
		}
	}

	if (textureId >= MAX_TEXTURES)
	{
		Error("Texture ID heap exhausted. Please increase MAX_TEXTURES\n");
	}

	Image *img = malloc(sizeof(Image));
	CheckAlloc(img);

	Asset *textureAsset = DecompressAsset(asset, false);
	size_t offset = 0;
	// TODO something like EXPECT_BYTES but it generates a missing tex instead of returning NULL
	if (textureAsset == NULL || textureAsset->type != ASSET_TYPE_TEXTURE)
	{
		GenFallbackImage(img);
	} else
	{
		if (textureAsset->typeVersion != TEXTURE_ASSET_VERSION)
		{
			LogError("Failed to load texture from asset due to version mismatch (got %d, expected %d)",
					 textureAsset->typeVersion,
					 TEXTURE_ASSET_VERSION);
			GenFallbackImage(img);
		} else
		{
			img->width = ReadSizeT(textureAsset->data, &offset);
			img->height = ReadSizeT(textureAsset->data, &offset);
			img->filter = ReadByte(textureAsset->data, &offset) != 0;
			img->repeat = ReadByte(textureAsset->data, &offset) != 0;
			img->mipmaps = ReadByte(textureAsset->data, &offset) != 0;
			const size_t pixelDataSize = img->width * img->height * sizeof(uint32_t);
			img->pixelData = malloc(pixelDataSize);
			CheckAlloc(img->pixelData);
			memcpy(img->pixelData, textureAsset->data + offset, pixelDataSize);
			uint32_t *pixels32 = (uint32_t *)img->pixelData;
			for (size_t i = 0; i < img->width * img->height; i++)
			{
				pixels32[i] = SDL_Swap32BE(pixels32[i]); // endianness is SO fun
			}
		}
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
		LogWarning("Texture ID heap is nearly exhausted! Only %zu slots remain.\n", MAX_TEXTURES - textureId);
	}

	if (textureAsset)
	{
		FreeAsset(textureAsset);
	}

	return img;
}

Image *RegisterFallbackImage()
{
	const char *asset = "_generic_fallback";
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		Image *img = images[i];
		if (img == NULL)
		{
			break;
		}
		if (strcmp(asset, img->name) == 0)
		{
			return img;
		}
	}

	if (textureId >= MAX_TEXTURES)
	{
		Error("Texture ID heap exhausted. Please increase MAX_TEXTURES\n");
	}

	Image *img = malloc(sizeof(Image));
	CheckAlloc(img);
	GenFallbackImage(img);

	img->id = textureId;

	const size_t nameLength = strlen(asset) + 1;
	img->name = malloc(nameLength);
	CheckAlloc(img->name);
	strncpy(img->name, asset, nameLength);

	images[textureId] = img;

	textureId++;

	if (textureId >= MAX_TEXTURES - 10)
	{
		LogWarning("Texture ID heap is nearly exhausted! Only %zu slots remain.\n", MAX_TEXTURES - textureId);
	}

	return img;
}

void DestroyTextureLoader()
{
	LogDebug("Cleaning up texture cache...\n");
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		if (images[i] != NULL)
		{
			free(images[i]->name);
			free(images[i]->pixelData);
			free(images[i]);
		}
	}
}
