//
// Created by droc101 on 4/26/2024.
//

#ifndef GAME_ASSETREADER_H
#define GAME_ASSETREADER_H

#include "../../defines.h"

/// The maximum number of textures that can be loaded in any one execution of the game
#define MAX_TEXTURES 512

/// The maximum number of models that can be loaded in any one execution of the game
#define MAX_MODELS 128

/**
 * Initialize the asset cache
 */
void AssetCacheInit();

/**
 * Invalidate the asset cache
 * @warning If anything still has a pointer to an asset, it will become invalid!
 */
void DestroyAssetCache();

/**
 * Decompress an asset and cache it
 * @param relPath The asset to decompress
 * @param cache Whether the asset should be cached
 * @return Decompressed asset, including header
 * @warning If the asset is not cached, you will have to pass it to @c FreeAsset. Otherwise,
 * it is kept around until program exit and automaitcally freed.
 */
Asset *DecompressAsset(const char *relPath, bool cache);

/**
 * Remove an asset from the cache
 * @param relPath The asset to decompress
 * @note Any pointers to this asset will become invalid.
 */
void RemoveAssetFromCache(const char *relPath);

/**
 * Load an image from disk, falling back to a cached version if possible
 * @param asset The asset to load the image from
 * @return The loaded image, or a 64x64 fallback image if it failed
 */
Image *LoadImage(const char *asset);

/**
 * Load a model from an asset
 * @param asset The asset to load the model from
 * @return The loaded model, or NULL if it failed
 */
ModelDefinition *LoadModel(const char *asset);

/**
 * Fetch a cached model from an ID
 * @param id The model ID to fetch
 * @return The model with the given ID
 */
extern ModelDefinition *GetModelFromId(uint id);

/**
 * Load a font from an asset
 * @param asset The asset to load the font from
 * @return The loaded font, or NULL if it failed
 * @note This pointer is not tracked and must be freed manually.
 */
Font *LoadFont(const char *asset);

/**
 * Free an asset
 * @param asset The asset to free
 * @warning Only use this on assets that are NOT cached.
 */
void FreeAsset(Asset *asset);

#define TEXTURE(assetName) ("texture/" assetName ".gtex")
#define MUSIC(assetName) ("audio/" assetName ".gmus")
#define SOUND(assetName) ("audio/" assetName ".gsnd")
#define LEVEL(assetName) ("level/" assetName ".gmap")
#define OGL_SHADER(assetName) ("glshader/" assetName ".gshd")
#define VK_FRAG(assetName) ("vkshader/" assetName ".gfrg")
#define VK_VERT(assetName) ("vkshader/" assetName ".gvrt")
#define MODEL(assetName) ("model/" assetName ".gmdl")
#define FONT(assetName) ("font/" assetName ".gfon")

#endif //GAME_ASSETREADER_H
