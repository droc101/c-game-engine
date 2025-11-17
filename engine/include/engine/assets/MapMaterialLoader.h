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
	char *name;
	uint32_t id;

	char *texture;
	ModelShader shader;
	SoundClass soundClass;
};

MapMaterial *LoadMapMaterial(const char *path);

#endif //GAME_MAPMATERIALLOADER_H
