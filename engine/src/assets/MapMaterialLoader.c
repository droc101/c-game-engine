//
// Created by droc101 on 11/16/25.
//

#include <engine/assets/AssetReader.h>
#include <engine/assets/DataReader.h>
#include <engine/assets/MapMaterialLoader.h>
#include <engine/structs/Asset.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint32_t mapMaterialId;
MapMaterial *mapMaterials[MAX_MAP_MATERIALS];

MapMaterial *LoadMapMaterial(const char *path)
{
	for (int i = 0; i < MAX_MAP_MATERIALS; i++)
	{
		MapMaterial *material = mapMaterials[i];
		if (material == NULL)
		{
			break;
		}
		if (strcmp(path, material->name) == 0)
		{
			return material;
		}
	}

	if (mapMaterialId >= MAX_MAP_MATERIALS)
	{
		Error("Map Material ID heap exhausted. Please increase MAX_MAP_MATERIALS\n");
	}

	MapMaterial *material = malloc(sizeof(MapMaterial));
	CheckAlloc(material);

	Asset *mapMaterialAsset = DecompressAsset(path, false, false);
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
	size_t bytesRemaining = mapMaterialAsset->size;
	size_t strLength = 0;

	material->texture = ReadStringSafe(mapMaterialAsset->data, &offset, mapMaterialAsset->size, &strLength);
	bytesRemaining -= sizeof(size_t);
	bytesRemaining -= strLength;
	EXPECT_BYTES(sizeof(float) * 2, bytesRemaining);
	offset += sizeof(float) * 2; // default scale is lvledit side only
	EXPECT_BYTES(2, bytesRemaining);
	material->shader = ReadByte(mapMaterialAsset->data, &offset);
	material->soundClass = ReadByte(mapMaterialAsset->data, &offset);

	material->id = mapMaterialId;

	const size_t nameLength = strlen(path);
	material->name = malloc(nameLength + 1);
	CheckAlloc(material->name);
	strncpy(material->name, path, nameLength);

	mapMaterials[mapMaterialId] = material;

	mapMaterialId++;

	if (mapMaterialId >= MAX_MAP_MATERIALS - 10)
	{
		LogWarning("Map Material ID heap is nearly exhausted! Only %zu slots remain.\n",
				   MAX_MAP_MATERIALS - mapMaterialId);
	}

	FreeAsset(mapMaterialAsset);

	return material;
}
