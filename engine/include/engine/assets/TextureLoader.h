//
// Created by droc101 on 7/23/25.
//

/**
 * @file TextureLoader.h
 * Texture asset loader & cache system
 */

#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <engine/structs/Asset.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/// Texture asset version number
#define TEXTURE_ASSET_VERSION 2

/// The maximum number of textures that can be loaded in any one execution of the game
#define MAX_TEXTURES 512

#if MAX_TEXTURES > 500000
#error "The required minimum for the maxPerStageDescriptorUpdateAfterBindSamplers limit is 500000, which should not be exceeded. Additionally, the upper two bits must remain unused, since they're used for texture metadata by the Vulkan code."
#endif

/// The name of the always-present missing texture
#define MISSING_TEXTURE_NAME "dynamic/missingtex"

typedef struct Image Image;

/// The format of the pixels in an image
typedef enum ImagePixelFormat
{
	/// RGBA, uint8_t per channel
	PIXEL_FORMAT_RGBA8,
	/// RGBA, Float16 per channel
	PIXEL_FORMAT_RGBA16F,
} ImagePixelFormat;

/// A loaded texture asset
struct Image
{
	/// The width of the image
	size_t width;
	/// The height of the image
	size_t height;
	/// The ID of the image. This is generated at runtime and not consistent between runs.
	uint32_t id;
	/// The pixel format of the image
	ImagePixelFormat pixelFormat;

	/// Whether to use filtering on this texture
	bool filter;
	/// Whether this texture repeats
	bool repeat;
	/// Whether to generate mipmaps for this texture
	bool mipmaps;
	/// Whether the image is fully opaque
	bool opaque;

	/// The name of the image
	char *name;
	/// The pixel data of the image
	uint8_t *pixelData;
};

/**
 * Generate a "missing texture"
 * @param src The image to populate
 */
void GenFallbackImage(Image *src);

/**
 * Load an image from an asset, without registering it
 * @param asset The asset to load from
 * @param image The image to load into
 * @return Success/failure
 */
bool LoadImageFromAsset(const Asset *asset, Image *image);

/**
 * Load an image from disk, falling back to a cached version if possible
 * @param asset The asset to load the image from
 * @return The loaded image, or a 64x64 fallback image if it failed
 */
Image *LoadImage(const char *asset);

/**
 * Get the missing texture
 */
Image *GetMissingTexture();

/**
 * Register an image
 * @param image The image to register
 * @return Whether the registration was expected
 * @note The image should already have its name set, but it will be given an ID by this function
 */
bool RegisterImage(Image *image);

/**
 * Initialize the texture loader
 * @private
 */
void InitTextureLoader();

/**
 * Destroy the texture loader and clean up all loaded textures
 * @private
 */
void DestroyTextureLoader();

/// @private
void DebugEntryTextureLoader();

#endif //TEXTURELOADER_H
