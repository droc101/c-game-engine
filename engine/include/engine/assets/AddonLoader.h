//
// Created by droc101 on 8/20/26.
//

#ifndef GAME_ADDONLOADER_H
#define GAME_ADDONLOADER_H

#include <engine/assets/GameConfigLoader.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <stdbool.h>

typedef enum AddonType
{
	ADDON_TYPE_ASSET_PACK,
} AddonType;

typedef struct Addon
{
	/// Unique identifier for this addon
	char *id;
	/// Path to this addon's folder on disk
	char *rootPath;
	/// Asset search path to use when this addon is enabled
	AssetPath assetPath;
	/// Icon texture name, do not use directly, use @c GetAddonIcon instead
	char *icon;

	/// Display name of this addon
	char *displayName;
	/// Description of this addon
	char *description;
	/// Type of this addon
	AddonType type;
} Addon;

extern List addons;
extern List enabledAddons;

/**
 * Initialize the addon loader
 */
void InitAddonLoader();

/**
 * Destroy the addon loader
 */
void DestroyAddonLoader();

/**
 * Apply enabled addon asset paths to the loaded game config. This does NOT trigger an asset reload, the caller must do that themselves
 */
void ApplyAddonAssetPaths();

/**
 * Clear addon icon paths, this should only be called during an asset reload
 */
void ClearAddonIcons();

/**
 * Get the icon name of an addon
 * @param addon The addon to get the icon for
 * @return The icon name
 */
char *GetAddonIcon(Addon *addon);

/**
 * Reset enabled addons
 */
void DefaultAddonSettings();

/**
 * Load enabled addon settings
 */
void LoadAddonSettings(ParamArray *from);

/**
 * Save enabled addon settings
 */
ParamArray SaveAddonSettings();

/**
 * Check if an addon is enabled
 */
bool IsAddonEnabled(Addon *addon);

/**
 * Enable or disable an addon
 */
void SetAddonEnabled(Addon *addon, bool enabled);

/**
 * Get an addon by its ID
 */
Addon *GetAddonById(const char *id);

#endif //GAME_ADDONLOADER_H
