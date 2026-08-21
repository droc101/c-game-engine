//
// Created by droc101 on 7/23/25.
//

#include <assert.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/debug/DPrint.h>
#include <engine/structs/Asset.h>
#include <engine/structs/Color.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint32_t textureId;
static Image *images[MAX_TEXTURES];

#define MISSING_TEX_SIZE 2
#define MISSING_TEX_COLOR_A 0xFF000000
#define MISSING_TEX_COLOR_B 0xFFFF00FF

static inline int ImageNameMatch(const void *imageName, const void *image)
{
	assert(image && *(const Image **)image);
	return strcmp(imageName, (*(const Image **)image)->name);
}

static inline int ImagesNameCompare(const void *img1, const void *img2)
{
	assert(img1 && img2);
	const Image *imageOne = *(const Image **)img1;
	if (imageOne == NULL)
	{
		return 1;
	}
	const Image *imageTwo = *(const Image **)img2;
	if (imageTwo == NULL)
	{
		return -1;
	}
	return strcmp(imageOne->name, imageTwo->name);
}

static Image *GetCachedImage(const char *name)
{
	Image **foundImage = bsearch(name, images, textureId, sizeof(Image *), ImageNameMatch);
	if (foundImage != NULL && *foundImage != NULL)
	{
		return *foundImage;
	}
	return NULL;
}

void GenFallbackImage(Image *src)
{
	src->width = MISSING_TEX_SIZE;
	src->height = MISSING_TEX_SIZE;
	src->filter = false;
	src->repeat = true;
	src->mipmaps = false;
	src->pixelFormat = PIXEL_FORMAT_RGBA8;
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

bool LoadImageFromAsset(const Asset *asset, Image *image)
{
	if (asset == NULL || asset->type != ASSET_TYPE_TEXTURE)
	{
		return false;
	}
	DataReader *reader = CreateDataReaderFromAsset(asset);
	if (asset->typeVersion != TEXTURE_ASSET_VERSION)
	{
		LogError("Failed to load texture from asset due to version mismatch (got %d, expected %d)\n",
				 asset->typeVersion,
				 TEXTURE_ASSET_VERSION);
		return false;
	}
	const size_t headerSize = (sizeof(size_t) * 2) + (sizeof(uint8_t) * 4);
	if (asset->size < headerSize)
	{
		LogError("Failed to load texture asset as it was the wrong size.\n");
		return false;
	}
	image->width = ReadSizeT(reader);
	image->height = ReadSizeT(reader);
	image->filter = ReadUint8(reader) != 0;
	image->repeat = ReadUint8(reader) != 0;
	image->mipmaps = ReadUint8(reader) != 0;
	image->pixelFormat = ReadUint8(reader);
	size_t pixelDataSize = image->width * image->height;
	if (image->pixelFormat == PIXEL_FORMAT_RGBA8)
	{
		pixelDataSize *= sizeof(uint32_t);
	} else
	{
		pixelDataSize *= sizeof(_Float16) * 4;
	}
	if (asset->size < headerSize + pixelDataSize)
	{
		LogError("Failed to load texture asset as it was the wrong size.\n");
		return false;
	}

	image->pixelData = malloc(pixelDataSize);
	CheckAlloc(image->pixelData);
	ReadBuffer(reader, pixelDataSize, image->pixelData);

	DestroyDataReader(reader);

	return true;
}

Image *LoadImage(const char *asset)
{
	Image *existingImage = GetCachedImage(asset);
	if (existingImage)
	{
		return existingImage;
	}

	if (textureId >= MAX_TEXTURES)
	{
		return GetMissingTexture();
	}

	Image *img = malloc(sizeof(Image));
	CheckAlloc(img);

	Asset *textureAsset = LoadAsset(asset, false, false);
	if (!LoadImageFromAsset(textureAsset, img))
	{
		GenFallbackImage(img);
	}

	const size_t nameLength = strlen(asset) + 1;
	img->name = malloc(nameLength);
	CheckAlloc(img->name);
	strncpy(img->name, asset, nameLength);

	if (!RegisterImage(img))
	{
		return GetMissingTexture();
	}

	if (textureAsset)
	{
		FreeAsset(textureAsset);
	}

	return img;
}

bool RegisterImage(Image *image)
{
	if (GetCachedImage(image->name))
	{
		return false;
	}

	if (textureId >= MAX_TEXTURES)
	{
		return false;
	}

	image->id = textureId;
	images[textureId] = image;

	textureId++;

	qsort(images, textureId, sizeof(Image *), ImagesNameCompare);

	if (textureId == MAX_TEXTURES)
	{
		LogError("Texture ID heap exhausted. Please increase MAX_TEXTURES\n");
	} else if (textureId >= MAX_TEXTURES - 10)
	{
		LogWarning("Texture ID heap is nearly exhausted! Only %zu slots remain.\n", MAX_TEXTURES - textureId);
	}

	return true;
}

Image *GetMissingTexture()
{
	Image *existing = GetCachedImage(MISSING_TEXTURE_NAME);
	if (existing)
	{
		return existing;
	}

	Image *img = malloc(sizeof(Image));
	img->name = strdup(MISSING_TEXTURE_NAME);
	CheckAlloc(img);
	GenFallbackImage(img);

	if (!RegisterImage(img))
	{
		Error("Failed to register missing texture!");
	}

	return img;
}

void InitTextureLoader()
{
	// ensures the missing texture is available as a fallback
	(void)GetMissingTexture();
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
			images[i] = NULL;
		}
	}
	textureId = 0;
}

void DPrintTextureLoader()
{
	DPrintF("Texture Cache: %zu/%zu textures", COLOR_WHITE, textureId, MAX_TEXTURES);
}
