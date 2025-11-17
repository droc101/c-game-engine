//
// Created by droc101 on 11/16/25.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/MapMaterialLoader.h>
#include <engine/structs/Asset.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

uint32_t mapMaterialId;
MapMaterial *mapMaterials[MAX_MAP_MATERIALS];

MapMaterial *LoadMapMaterial(const char *path)
{
	for (int i = 0; i < MAX_MAP_MATERIALS; i++)
	{
		MapMaterial *mat = mapMaterials[i];
		if (mat == NULL)
		{
			break;
		}
		if (strncmp(path, mat->name, 80) == 0)
		{
			return mat;
		}
	}

	if (mapMaterialId >= MAX_MAP_MATERIALS)
	{
		Error("Map Material ID heap exhausted. Please increase MAX_MAP_MATERIALS\n");
	}

	MapMaterial *mat = malloc(sizeof(MapMaterial));
	CheckAlloc(mat);

	Asset *mapMaterialAsset = DecompressAsset(path, false);
	if (mapMaterialAsset == NULL || mapMaterialAsset->type != ASSET_TYPE_MAP_MATERIAL)
	{
		return NULL;
	}

	if (mapMaterialAsset->typeVersion != MAP_MATERIAL_ASSET_VERSION)
	{
		LogError("Failed to load map material from asset due to version mismatch (got %d, expected %d)",
				 mapMaterialAsset->typeVersion,
				 MAP_MATERIAL_ASSET_VERSION);
		return NULL;
	}

	size_t offset = 0;

	mat->texture = ReadStringSafe(mapMaterialAsset->data, &offset, mapMaterialAsset->size, NULL);
	offset += sizeof(float) * 2;
	mat->shader = ReadByte(mapMaterialAsset->data, &offset);
	mat->soundClass = ReadByte(mapMaterialAsset->data, &offset);

	mat->id = mapMaterialId;

	const size_t nameLength = strlen(path) + 1;
	mat->name = malloc(nameLength);
	CheckAlloc(mat->name);
	strncpy(mat->name, path, nameLength);

	mapMaterials[mapMaterialId] = mat;

	mapMaterialId++;

	if (mapMaterialId >= MAX_MAP_MATERIALS - 10)
	{
		LogWarning("Map Material ID heap is nearly exhausted! Only %zu slots remain.\n",
				   MAX_MAP_MATERIALS - mapMaterialId);
	}

	FreeAsset(mapMaterialAsset);

	return mat;
}
