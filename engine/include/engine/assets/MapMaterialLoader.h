//
// Created by droc101 on 11/16/25.
//

#ifndef GAME_MAPMATERIALLOADER_H
#define GAME_MAPMATERIALLOADER_H

#include <engine/assets/ModelLoader.h>
#include <stdint.h>

#define MAP_MATERIAL_ASSET_VERSION 1

/// The maximum number of map materials that can be loaded in any one execution of the game
#define MAX_MAP_MATERIALS 512

typedef enum SoundClass SoundClass;
typedef struct MapMaterial MapMaterial;

enum SoundClass
{
	SOUND_CLASS_DEFAULT
};

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
};

/**
 * Load a map material from an asset
 * @param path The asset path
 * @return The map material, or NULL on error
 */
MapMaterial *LoadMapMaterial(const char *path);

#endif //GAME_MAPMATERIALLOADER_H
