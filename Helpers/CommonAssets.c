//
// Created by droc101 on 7/7/2024.
//

#include "CommonAssets.h"
#include "Core/AssetLoaders/FontLoader.h"
#include "Core/AssetReader.h"

Font *smallFont;
Font *largeFont;

void InitCommonAssets()
{
	smallFont = LoadFont(FONT("small_font"));
	largeFont = LoadFont(FONT("large_font"));
}

void DestroyCommonAssets()
{
	FreeFont(smallFont);
	FreeFont(largeFont);
}
