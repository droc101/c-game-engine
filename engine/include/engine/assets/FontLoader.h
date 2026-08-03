//
// Created by droc101 on 7/23/25.
//

#ifndef FONTLOADER_H
#define FONTLOADER_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

/// The maximum number of fonts that can be loaded in any one execution of the game
#define MAX_FONTS 16

#define FONT_ASSET_VERSION 1

typedef struct Font Font;

struct Font
{
	char *name;

	/// The texture width of one character
	uint8_t width;
	/// The texture height (including below baseline)
	uint8_t textureHeight;
	/// The pixel coordinate of the baseline
	uint8_t baseline;
	/// The pixels between characters
	uint8_t charSpacing;
	/// The pixels between lines
	uint8_t lineSpacing;
	/// The width of a space character
	uint8_t spaceWidth;
	/// The default size of the font, used for calculating scale
	uint8_t defaultSize;
	/// The number of characters in the font
	uint8_t charCount;
	/// Whether this font only contains uppercase characters
	bool uppercaseOnly;

	/// The texture this font uses (fully qualified)
	char *texture;
	/// The width of each character, index directly by the character
	uint8_t charWidths[255];
	float charStartUVs[255];
	float charEndUVs[255];
};

/**
 * Load a font from an asset
 * @param asset The asset to load the font from
 * @return The loaded font, or NULL if it failed
 * @note This pointer is not tracked and must be freed manually.
 */
Font *LoadFont(const char *asset);

void DestroyFontLoader();

void DPrintFontLoader();

#endif //FONTLOADER_H
