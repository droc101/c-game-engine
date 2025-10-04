//
// Created by droc101 on 7/23/25.
//

#include "FontLoader.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../Structs/Asset.h"
#include "../AssetReader.h"
#include "../DataReader.h"
#include "../Error.h"
#include "../Logging.h"
#include "TextureLoader.h"

Font *GenerateFallbackFont()
{
	Font *font = malloc(sizeof(Font));
	CheckAlloc(font);
	font->width = 16;
	font->textureHeight = 16;
	font->baseline = 16;
	font->charSpacing = 1;
	font->lineSpacing = 1;
	font->spaceWidth = 8;
	font->defaultSize = 16;
	font->uppercaseOnly = false;
	font->texture = calloc(strlen("_generic_fallback") + 1, sizeof(char));
	CheckAlloc(font->texture);
	strcpy(font->texture, "_generic_fallback");
	font->image = RegisterFallbackImage();
	font->charCount = 255;
	memset(font->indices, 0, 255);
	memset(font->charWidths, 0, 255);
	for (int i = 0; i < font->charCount; i++)
	{
		font->indices[i] = i;
		font->charWidths[i] = 16;
	}
	return font;
}

Font *LoadFont(const char *asset)
{
	Asset *assetData = DecompressAsset(asset, false);
	if (assetData == NULL)
	{
		LogError("Failed to load font from asset, asset was NULL!\n");
		return GenerateFallbackFont();
	}
	if (assetData->typeVersion != FONT_ASSET_VERSION)
	{
		LogError("Failed to load font from asset due to version mismatch (got %d, expected %d)\n",
				 assetData->typeVersion,
				 FONT_ASSET_VERSION);
		return GenerateFallbackFont();
	}
	const size_t baseSize = (sizeof(uint8_t) * 8) + sizeof(bool);
	if (assetData->size < baseSize)
	{
		LogError("Failed to load font from asset due to size mismatch (got %d bytes, expected at least %d bytes)\n",
				 assetData->size,
				 baseSize);
		return GenerateFallbackFont();
	}
	Font *font = malloc(sizeof(Font));
	CheckAlloc(font);
	size_t offset = 0;
	font->width = ReadByte(assetData->data, &offset);
	font->textureHeight = ReadByte(assetData->data, &offset);
	font->baseline = ReadByte(assetData->data, &offset);
	font->charSpacing = ReadByte(assetData->data, &offset);
	font->lineSpacing = ReadByte(assetData->data, &offset);
	font->spaceWidth = ReadByte(assetData->data, &offset);
	font->defaultSize = ReadByte(assetData->data, &offset);
	font->uppercaseOnly = ReadByte(assetData->data, &offset) != 0;
	size_t fontTextureLength = 0;
	char *fontTexture = ReadStringSafe(assetData->data, &offset, assetData->size, &fontTextureLength);
	if (!fontTexture)
	{
		LogError("Failed to load font from asset (unable to read texture string)\n");
		free(font);
		return GenerateFallbackFont();
	}
	fontTextureLength += strlen(TEXTURE(""));
	font->texture = calloc(fontTextureLength, sizeof(char));
	snprintf(font->texture, fontTextureLength, TEXTURE("%s"), fontTexture);
	free(fontTexture);
	font->image = LoadImage(font->texture);
	font->charCount = ReadByte(assetData->data, &offset);
	memset(font->indices, 0, 255);
	memset(font->charWidths, 0, 255);
	for (int i = 0; i < font->charCount; i++)
	{
		const char chr = (char)ReadByte(assetData->data, &offset);
		const uint8_t width = ReadByte(assetData->data, &offset);
		font->indices[(int)chr] = i;
		font->charWidths[(int)chr] = width;
	}
	FreeAsset(assetData);

	return font;
}

void FreeFont(Font *font)
{
	free(font->texture);
	free(font);
}
