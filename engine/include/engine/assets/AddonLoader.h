//
// Created by droc101 on 8/20/26.
//

#ifndef GAME_ADDONLOADER_H
#define GAME_ADDONLOADER_H

#include <engine/assets/GameConfigLoader.h>
#include <engine/structs/List.h>

typedef enum AddonType
{
	ADDON_TYPE_ASSET_PACK,
} AddonType;

typedef struct Addon
{
	char *id;
	char *rootPath;
	AssetPath assetPath;
	char *icon;

	char *displayName;
	char *description;
	AddonType type;
} Addon;

extern List addons;

void InitAddonLoader();

void DestroyAddonLoader();

void ApplyAddonAssetPaths();

#endif //GAME_ADDONLOADER_H
