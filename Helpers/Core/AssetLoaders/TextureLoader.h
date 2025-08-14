//
// Created by droc101 on 7/23/25.
//

#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TEXTURE_ASSET_VERSION 1

/// The maximum number of textures that can be loaded in any one execution of the game
#define MAX_TEXTURES 512

typedef struct Image Image;

struct Image
{
	/// The width of the image
	size_t width;
	/// The height of the image
	size_t height;
	/// The ID of the image. This is generated at runtime and not consistent between runs.
	uint32_t id;

	bool filter;
	bool repeat;
	bool mipmaps;

	/// The name of the image
	char *name;
	/// The pixel data of the image
	uint8_t *pixelData;
};

void GenFallbackImage(Image *src);

/**
 * Load an image from disk, falling back to a cached version if possible
 * @param asset The asset to load the image from
 * @return The loaded image, or a 64x64 fallback image if it failed
 */
Image *LoadImage(const char *asset);

Image *RegisterFallbackImage();

void DestroyTextureLoader();

#endif //TEXTURELOADER_H
