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
 * @return Decompressed asset, including header
 * @note The asset is cached
 */
Asset *DecompressAsset(const char *relPath);

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
ModelDefinition *GetModelFromId(uint id);

/**
 * Load a font from an asset
 * @param asset The asset to load the font from
 * @return The loaded font, or NULL if it failed
 * @note This pointer is not tracked and must be freed manually.
 */
Font *LoadFont(const char *asset);

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
