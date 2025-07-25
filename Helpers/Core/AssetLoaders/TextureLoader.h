//
// Created by droc101 on 7/23/25.
//

#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include "../../../defines.h"

/// The maximum number of textures that can be loaded in any one execution of the game
#define MAX_TEXTURES 512

void GenFallbackImage(Image *src);

/**
 * Load an image from disk, falling back to a cached version if possible
 * @param asset The asset to load the image from
 * @return The loaded image, or a 64x64 fallback image if it failed
 */
Image *LoadImage(const char *asset);

void DestroyTextureLoader();

#endif //TEXTURELOADER_H
