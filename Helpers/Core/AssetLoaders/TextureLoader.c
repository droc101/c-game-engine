//
// Created by droc101 on 7/23/25.
//

#include "TextureLoader.h"
#include "../Error.h"
#include "../AssetReader.h"
#include "../DataReader.h"
#include "../Logging.h"
#include <SDL_endian.h>

uint textureId;
Image *images[MAX_TEXTURES];

void GenFallbackImage(Image *src)
{
	src->width = 64;
	src->height = 64;
	src->filter = false;
	src->repeat = true;
	src->mipmaps = false;
	const size_t pixelDataSize = 64 * 64 * sizeof(uint32_t);
	src->pixelData = malloc(pixelDataSize);
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
		Error("Texture ID heap exhausted. Please increase MAX_TEXTURES\n");
	}

	Image *img = malloc(sizeof(Image));
	CheckAlloc(img);

	Asset *textureAsset = DecompressAsset(asset, false);
	size_t offset = 0;
	if (textureAsset == NULL || textureAsset->type != ASSET_TYPE_TEXTURE)
	{
		GenFallbackImage(img);
	} else
	{
		if (textureAsset->typeVersion != TEXTURE_ASSET_VERSION)
		{
			LogError("Failed to load font from asset due to version mismatch (got %d, expected %d)", textureAsset->typeVersion, TEXTURE_ASSET_VERSION);
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
			memcpy(img->pixelData, textureAsset->data + offset, pixelDataSize);
			uint32_t *pixels32 = (uint32_t*)img->pixelData;
			for (size_t i = 0; i < img->width * img->height; i++)
			{
				pixels32[i] = SDL_SwapBE32(pixels32[i]); // endianness is SO fun
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
		LogWarning("Texture ID heap is nearly exhausted! Only %lu slots remain.\n", MAX_TEXTURES - textureId);
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
		if (strncmp(asset, img->name, 80) == 0)
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
		LogWarning("Texture ID heap is nearly exhausted! Only %lu slots remain.\n", MAX_TEXTURES - textureId);
	}

	return img;
}

void DestroyTextureLoader()
{
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
