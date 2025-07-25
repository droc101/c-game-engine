//
// Created by droc101 on 7/23/25.
//

#include "FontLoader.h"
#include "../AssetReader.h"
#include "../DataReader.h"
#include "../Error.h"
#include "../Logging.h"
#include "TextureLoader.h"

Font *LoadFont(const char *asset)
{
	Asset *assetData = DecompressAsset(asset, false);
	if (assetData == NULL)
	{
		LogError("Failed to load font from asset, asset was NULL!\n");
		Error("Failed to load font!");
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
	const size_t texturePathLength = ReadSizeT(assetData->data, &offset);
	font->texture = calloc(texturePathLength + strlen("texture/.gtex"), sizeof(char));
	snprintf(font->texture, 80, "texture/%s.gtex", (char*)assetData->data + offset);
	font->image = LoadImage(font->texture);
	offset += texturePathLength;
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
