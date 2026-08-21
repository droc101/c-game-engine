//
// Created by droc101 on 8/20/26.
//

#include <dirent.h>
#include <engine/assets/AddonLoader.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/assets/KvlFile.h>
#include <engine/assets/TextureLoader.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Logging.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ADDONS_PATH "addons"
#define ADDON_CONFIG_FILENAME "addon.kvl"
#define ADDON_ASSETS_FOLDER "assets"

List addons;

static Addon *LoadAddon(const char *path, const char *id)
{
	const size_t config_path_length = strlen(path) + strlen(ADDON_CONFIG_FILENAME) + 2;
	char *config_filename = malloc(config_path_length);
	snprintf(config_filename, config_path_length, "%s/%s", path, ADDON_CONFIG_FILENAME);
	KvList config;
	if (!ReadKvlFile(config_filename, config))
	{
		free(config_filename);
		return NULL;
	}
	free(config_filename);
	Addon *addon = malloc(sizeof(Addon));
	addon->id = strdup(id);
	addon->rootPath = strdup(path);
	addon->displayName = strdup(KvGetString(config, "display_name", "Unknown Addon"));
	addon->description = strdup(KvGetString(config, "description", ""));
	addon->type = KvGetByte(config, "type", ADDON_TYPE_ASSET_PACK);
	addon->icon = strdup(MISSING_TEXTURE_NAME); // TODO load icon

	KvListDestroy(config);

	const size_t assets_path_length = strlen(path) + strlen(ADDON_ASSETS_FOLDER) + 2;
	char *assets_path = malloc(assets_path_length);
	snprintf(assets_path, assets_path_length, "%s/%s", path, ADDON_ASSETS_FOLDER);

	addon->assetPath.type = ABSOLUTE_PATH;
	addon->assetPath.flags = ASSET_PATH_RUNTIME_LOADED;
	addon->assetPath.path = CanonicalFilePath(assets_path);
	free(assets_path);

	return addon;
}

static void FreeAddon(Addon *addon)
{
	free(addon->id);
	free(addon->assetPath.path);
	free(addon->description);
	free(addon->displayName);
	free(addon->icon);
	free(addon->rootPath);
}

static void RescanAddons()
{
	for (size_t i = 0; i < addons.length; i++)
	{
		FreeAddon(ListGetPointer(addons, i));
	}
	ListFreeOnlyContents(addons);
	ListClear(addons);

	DIR *dir = opendir(ADDONS_PATH);
	if (dir == NULL)
	{
		LogError("Failed to open addons directory: %s\n", strerror(errno));
		return;
	}

	const struct dirent *ent = readdir(dir);
	while (ent != NULL)
	{
		if (ent->d_name[0] != '.' && ent->d_type == DT_DIR)
		{
			const size_t path_length = strlen(ADDONS_PATH) + 1 + strlen(ent->d_name) + 1;
			char *path = malloc(path_length);
			snprintf(path, path_length, "%s/%s", ADDONS_PATH, ent->d_name);
			Addon *addon = LoadAddon(path, ent->d_name);
			LogInfo("Loaded addon \"%s\"\n", addon->id);
			if (addon)
			{
				ListAdd(addons, addon);
			}
			free(path);
		}
		ent = readdir(dir);
	}
	closedir(dir);
}

void InitAddonLoader()
{
	ListInit(addons, LIST_POINTER);
	RescanAddons();
	ApplyAddonAssetPaths();
}

static void RemoveAddonAssetPaths()
{
	for (size_t i = gameConfig.assetPaths.length - 1; i > 0; i--)
	{
		const AssetPath *path = ListGetPointer(gameConfig.assetPaths, i);
		if ((path->flags & ASSET_PATH_RUNTIME_LOADED) != 0)
		{
			ListRemoveAt(gameConfig.assetPaths, i);
		}
	}
}

void DestroyAddonLoader()
{
	RemoveAddonAssetPaths();
	for (size_t i = 0; i < addons.length; i++)
	{
		FreeAddon(ListGetPointer(addons, i));
	}
	ListAndContentsFree(addons);
}

void ApplyAddonAssetPaths()
{
	RemoveAddonAssetPaths();

	for (size_t i = 0; i < addons.length; i++)
	{
		Addon *addon = ListGetPointer(addons, i);
		ListInsertAfter(gameConfig.assetPaths, 0, &addon->assetPath);
	}
}
