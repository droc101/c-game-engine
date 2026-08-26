//
// Created by droc101 on 8/20/26.
//

#include <dirent.h>
#include <engine/assets/AddonLoader.h>
#include <engine/assets/AssetReader.h>
#include <engine/assets/GameConfigLoader.h>
#include <engine/assets/KvlFile.h>
#include <engine/assets/TextureLoader.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/structs/Asset.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Logging.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ADDONS_PATH "addons"
#define ADDON_CONFIG_FILENAME "addon.kvl"
#define ADDON_ICON_FILENAME "icon.gtex"
#define ADDON_ASSETS_FOLDER "assets"
#define ADDON_ICON_PREFIX "dynamic/addon_"

List addons;

static bool enabledListCreated = false;
List enabledAddons;

void ClearAddonIcons()
{
	for (size_t i = 0; i < addons.length; i++)
	{
		Addon *a = ListGetPointer(addons, i);
		free(a->icon);
		a->icon = NULL;
	}
}

char *GetAddonIcon(Addon *addon)
{
	if (addon->icon)
	{
		return addon->icon;
	}

	const size_t iconRegisteredNameLength = strlen(ADDON_ICON_PREFIX) + strlen(addon->id) + 1;
	char *iconRegisteredName = malloc(iconRegisteredNameLength);
	snprintf(iconRegisteredName, iconRegisteredNameLength, ADDON_ICON_PREFIX "%s", addon->id);

	const size_t iconFilenameLength = strlen(addon->rootPath) + 1 + strlen(ADDON_ICON_FILENAME) + 1;
	char *iconFilename = malloc(iconFilenameLength);
	snprintf(iconFilename, iconFilenameLength, "%s/" ADDON_ICON_FILENAME, addon->rootPath);

	Image *icon = malloc(sizeof(Image));

	FILE *file = fopen(iconFilename, "r");
	free(iconFilename);
	if (file)
	{
		Asset *iconAsset = LoadAssetFromFile(file);
		if (!LoadImageFromAsset(iconAsset, icon))
		{
			GenFallbackImage(icon);
		}
		if (iconAsset)
		{
			FreeAsset(iconAsset);
		}
	} else
	{
		GenFallbackImage(icon);
	}

	icon->name = iconRegisteredName;
	RegisterImage(icon);
	addon->icon = strdup(iconRegisteredName);
	return addon->icon;
}

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
	addon->icon = NULL;

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
		if (ent->d_name[0] != '.')
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
	if (!enabledListCreated)
	{
		ListInit(enabledAddons, LIST_POINTER);
		enabledListCreated = true;
	}
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
	ListAndContentsFree(enabledAddons);
}

void ApplyAddonAssetPaths()
{
	RemoveAddonAssetPaths();

	for (size_t i = enabledAddons.length - 1; i != SIZE_MAX; i--)
	{
		Addon *addon = GetAddonById(ListGetPointer(enabledAddons, i));
		ListInsertAfter(gameConfig.assetPaths, 0, &addon->assetPath);
	}
}

void DefaultAddonSettings()
{
	if (!enabledListCreated)
	{
		ListInit(enabledAddons, LIST_POINTER);
		enabledListCreated = true;
	}
	ListClear(enabledAddons);
}

void LoadAddonSettings(ParamArray *from)
{
	if (!enabledListCreated)
	{
		ListInit(enabledAddons, LIST_POINTER);
		enabledListCreated = true;
	}
	for (size_t i = 0; i < from->length; i++)
	{
		const Param *p = &from->data[i];
		if (p->type == PARAM_TYPE_STRING)
		{
			ListAdd(enabledAddons, strdup(p->stringValue));
		}
	}
}

ParamArray SaveAddonSettings()
{
	ParamArray arr;
	arr.length = enabledAddons.length;
	arr.data = calloc(arr.length, sizeof(Param));
	for (size_t i = 0; i < arr.length; i++)
	{
		Param *p = &arr.data[i];
		p->type = PARAM_TYPE_STRING;
		p->stringValue = strdup(ListGetPointer(enabledAddons, i));
	}
	return arr;
}

bool IsAddonEnabled(Addon *addon)
{
	for (size_t i = 0; i < enabledAddons.length; i++)
	{
		if (strcmp(ListGetPointer(enabledAddons, i), addon->id) == 0)
		{
			return true;
		}
	}
	return false;
}

void SetAddonEnabled(Addon *addon, const bool enabled)
{
	if (enabled)
	{
		if (!IsAddonEnabled(addon))
		{
			ListAdd(enabledAddons, strdup(addon->id));
		}
	} else
	{
		for (size_t i = 0; i < enabledAddons.length; i++)
		{
			char *enabledId = ListGetPointer(enabledAddons, i);
			if (strcmp(enabledId, addon->id) == 0)
			{
				free(enabledId);
				ListRemoveAt(enabledAddons, i);
			}
		}
	}
}

Addon *GetAddonById(const char *id)
{
	for (size_t i = 0; i < addons.length; i++)
	{
		Addon *a = ListGetPointer(addons, i);
		if (strcmp(a->id, id) == 0)
		{
			return a;
		}
	}
	return NULL;
}

