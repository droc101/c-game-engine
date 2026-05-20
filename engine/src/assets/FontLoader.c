//
// Created by droc101 on 7/23/25.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/FontLoader.h>
#include <engine/assets/TextureLoader.h>
#include <engine/structs/Asset.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	Asset *assetData = LoadAsset(asset, false, false);
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
	DataReader *reader = CreateDataReaderFromAsset(assetData);
	size_t bytesRemaining = assetData->size;
	EXPECT_BYTES(8, bytesRemaining);

	font->width = ReadUint8(reader);
	font->textureHeight = ReadUint8(reader);
	font->baseline = ReadUint8(reader);
	font->charSpacing = ReadUint8(reader);
	font->lineSpacing = ReadUint8(reader);
	font->spaceWidth = ReadUint8(reader);
	font->defaultSize = ReadUint8(reader);
	font->uppercaseOnly = ReadUint8(reader) != 0;
	size_t fontTextureLength = 0;
	char *fontTexture = ReadStringSafe(reader, &fontTextureLength);
	if (!fontTexture)
	{
		LogError("Failed to load font from asset (unable to read texture string)\n");
		free(font);
		DestroyDataReader(reader);
		return GenerateFallbackFont();
	}
	bytesRemaining -= fontTextureLength;
	bytesRemaining += sizeof(size_t);
	fontTextureLength += strlen(TEXTURE(""));
	font->texture = calloc(fontTextureLength, sizeof(char));
	CheckAlloc(font->texture);
	snprintf(font->texture, fontTextureLength, TEXTURE("%s"), fontTexture);
	free(fontTexture);
	font->image = LoadImage(font->texture);
	font->charCount = ReadUint8(reader);
	memset(font->indices, 0, 255);
	memset(font->charWidths, 0, 255);
	EXPECT_BYTES(2 * font->charCount, bytesRemaining);
	for (int i = 0; i < font->charCount; i++)
	{
		const char chr = (char)ReadUint8(reader);
		const uint8_t width = ReadUint8(reader);
		font->indices[(int)chr] = i;
		font->charWidths[(int)chr] = width;
	}
	DestroyDataReader(reader);
	FreeAsset(assetData);

	return font;
}

void FreeFont(Font *font)
{
	free(font->texture);
	free(font);
}
