//
// Created by droc101 on 7/23/25.
//

/**
 * @file FontLoader.h
 * Font asset loading & cache system
 */

#ifndef FONTLOADER_H
#define FONTLOADER_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

/// The maximum number of fonts that can be loaded in any one execution of the game
#define MAX_FONTS 16

/// Font asset version number
#define FONT_ASSET_VERSION 1

typedef struct Font Font;

/// A loaded Font asset
struct Font
{
	/// The name of this font
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
	/// The horizontal UV that each character starts at
	float charStartUVs[255];
	/// The horizontal UV that each character ends at
	float charEndUVs[255];
};

/**
 * Load a font from an asset
 * @param asset The asset to load the font from
 * @return The loaded font
 */
Font *LoadFont(const char *asset);

/**
 * Clean up loaded fonts
 * @private
 */
void DestroyFontLoader();

/// @private
void DebugEntryFontLoader();

#endif //FONTLOADER_H
