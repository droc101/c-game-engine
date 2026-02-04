//
// Created by droc101 on 7/23/25.
//

#ifndef FONTLOADER_H
#define FONTLOADER_H

#include <engine/assets/TextureLoader.h>
#include <stdbool.h>
#include <stdint.h>

#define FONT_ASSET_VERSION 1

typedef struct Font Font;

struct Font
{
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
	/// The index of the character in the texture
	uint8_t indices[255];
	/// The width of each character, index directly by the character
	uint8_t charWidths[255];

	/// The image loaded from the texture
	Image *image;
} __attribute__((packed));

/**
 * Generate a generic fallback font
 */
Font *GenerateFallbackFont();

/**
 * Load a font from an asset
 * @param asset The asset to load the font from
 * @return The loaded font, or NULL if it failed
 * @note This pointer is not tracked and must be freed manually.
 */
Font *LoadFont(const char *asset);

/**
 * Free a font asset
 * @param font The font to free
 */
void FreeFont(Font *font);

#endif //FONTLOADER_H
