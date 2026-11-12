//
// Created by droc101 on 11/16/25.
//

/**
 * @file MapMaterialLoader.h
 * Map material asset loader & cache system
 */

#ifndef GAME_MAPMATERIALLOADER_H
#define GAME_MAPMATERIALLOADER_H

#include <engine/assets/ModelLoader.h>
#include <stdbool.h>
#include <stdint.h>

/// Map material asset version number
#define MAP_MATERIAL_ASSET_VERSION 1

/// The maximum number of map materials that can be loaded in any one execution of the game
#define MAX_MAP_MATERIALS 512

typedef struct MapMaterial MapMaterial;

/// The type of sound this material makes
typedef enum SoundClass
{
	SOUND_CLASS_DEFAULT,
} SoundClass;

/// A loaded map material asset
struct MapMaterial
{
	/// The name/path of this material
	char *name;
	/// The ID of this material
	uint32_t id;

	/// The texture path of this material
	char *texture;
	/// The shader this material uses
	ModelShader shader;
	/// The sound class this material uses
	SoundClass soundClass;
	/// If this material should cast shadows or not
	bool castsShadows;
};

/**
 * Load a map material from an asset
 * @param path The asset path
 * @return The map material, or a fallback on error
 */
MapMaterial *LoadMapMaterial(const char *path);

/**
 * Clean up loaded map materials
 * @private
 */
void DestroyMapMaterialLoader();

#endif //GAME_MAPMATERIALLOADER_H
