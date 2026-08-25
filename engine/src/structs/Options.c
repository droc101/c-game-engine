//
// Created by droc101 on 10/27/24.
//

#include <engine/assets/AddonLoader.h>
#include <engine/assets/KvlFile.h>
#include <engine/debug/DebugEntryManager.h>
#include <engine/helpers/Arguments.h>
#include <engine/structs/ControlOptions.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Options.h>
#include <engine/structs/VideoPreset.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stddef.h>

#define OPTIONS_FILE "options.kvl"

static void DefaultOptions(Options *options)
{
	options->enableDiscordRpc = true;
	options->musicVolume = 1.0f;
	options->sfxVolume = 1.0f;
	options->uiVolume = 1.0f;
	options->masterVolume = 1.0f;
	options->fullscreen = false;
	options->cameraSpeed = 1;
	options->rumbleStrength = 1.0f;
	options->controllerDeadzone = 0.1f;
	options->controllerSwapOkCancel = false;
	options->preferWayland = true;
	options->fov = 90.0f;
	options->maxFps = 0;
	options->preferredGpuType = GPU_TYPE_DEDICATED;
#ifdef BUILDSTYLE_DEBUG
	options->vsync = false;
	options->limitFpsWhenUnfocused = false;
#else
	options->vsync = true;
	options->limitFpsWhenUnfocused = true;
#endif

	ApplyVideoPreset(options, VIDEO_PRESET_MEDIUM);
	DefaultControls();
	DefaultDebugEntrySettings();
	DefaultAddonSettings();
}

static bool ValidateOptions(const Options *options)
{
	// ignore controller mode
	if (options->cameraSpeed < 0.01 || options->cameraSpeed > 2.00)
	{
		return false;
	}
	if (options->rumbleStrength < 0.0 || options->rumbleStrength > 1.00)
	{
		return false;
	}
	// ignore invert h/v and swap a/b

	// ignore fullscreen,vsync
	if (options->msaa > MSAA_8X)
	{
		return false;
	}
	// ignore mipmaps, wayland/x11, bg fps limit
	if (options->lodMultiplier < 0.5 || options->lodMultiplier > 2.0)
	{
		return false;
	}
	if (options->fov < 30 || options->fov > 120)
	{
		return false;
	}
	if (options->anisotropy > ANISOTROPY_16X)
	{
		return false;
	}
	if (options->maxFps % 10 != 0 || options->maxFps > 500)
	{
		return false;
	}


	if (options->sfxVolume < 0 || options->sfxVolume > 1)
	{
		return false;
	}
	if (options->musicVolume < 0 || options->musicVolume > 1)
	{
		return false;
	}
	if (options->uiVolume < 0 || options->uiVolume > 1)
	{
		return false;
	}
	if (options->masterVolume < 0 || options->masterVolume > 1)
	{
		return false;
	}
	return true;
}

void LoadOptions(Options *options)
{
	if (HasCliArg("--reset-options"))
	{
		LogInfo("Resetting options...");
		DefaultOptions(options);
		return;
	}

	KvList list;
	if (ReadKvlFile(OPTIONS_FILE, list))
	{
		options->enableDiscordRpc = KvGetBool(list, "enable_discord_rpc", true);
		options->cameraSpeed = KvGetFloat(list, "camera_speed", 1.0f);
		options->rumbleStrength = KvGetFloat(list, "rumble_strength", 1.0f);
		options->controllerDeadzone = KvGetFloat(list, "controller_deadzone", 0.1f);
		options->controllerSwapOkCancel = KvGetBool(list, "controller_swap_ok_cancel", false);

		KvList controls;
		if (KvGetList(list, "controls", controls))
		{
			LoadControls(controls);
			KvListDestroy(controls);
		} else
		{
			DestroyControls();
		}

		KvList debugOptions;
		if (KvGetList(list, "debug_options", debugOptions))
		{
			LoadDebugEntrySettings(debugOptions);
			KvListDestroy(debugOptions);
		} else
		{
			DefaultDebugEntrySettings();
		}

		ParamArray *enabledAddons = KvGetArray(list, "enabled_addons");
		if (enabledAddons)
		{
			LoadAddonSettings(enabledAddons);
		} else
		{
			DefaultAddonSettings();
		}

		options->fullscreen = KvGetBool(list, "fullscreen", false);
		options->vsync = KvGetBool(list, "vsync", true);
		options->preferWayland = KvGetBool(list, "prefer_wayland", true);
		options->limitFpsWhenUnfocused = KvGetBool(list, "limit_fps_when_unfocused", true);
		options->fov = KvGetFloat(list, "fov", 90.0f);
		options->maxFps = KvGetInt(list, "max_fps", 0);
		options->preferredGpuType = KvGetByte(list, "preferred_gpu_type", GPU_TYPE_DEDICATED);

		if (KvHas(list, "video_preset", PARAM_TYPE_BYTE))
		{
			const VideoPreset preset = KvGetByte(list, "video_preset", VIDEO_PRESET_MEDIUM);
			ApplyVideoPreset(options, preset);
		} else
		{
			options->msaa = KvGetByte(list, "msaa", MSAA_4X);
			options->mipmaps = KvGetBool(list, "mipmaps", true);
			options->lodMultiplier = KvGetFloat(list, "lod_multiplier", 1.0f);
			options->anisotropy = KvGetByte(list, "anisotropy", ANISOTROPY_16X);
		}

		options->musicVolume = KvGetFloat(list, "music_volume", 1.0f);
		options->sfxVolume = KvGetFloat(list, "sfx_volume", 1.0f);
		options->uiVolume = KvGetFloat(list, "ui_volume", 1.0f);
		options->masterVolume = KvGetFloat(list, "master_volume", 1.0f);

		KvListDestroy(list);
	} else
	{
		LogWarning("Options file failed to load, defaults will be used\n");
		DefaultOptions(options);
	}

	if (!ValidateOptions(options))
	{
		LogWarning("Options file is invalid, using defaults\n");
		DefaultOptions(options);
	}
}

void SaveOptions(Options *options)
{
	KvList list;
	KvListCreate(list);

	KvSetBool(list, "enable_discord_rpc", options->enableDiscordRpc);

	KvSetFloat(list, "camera_speed", options->cameraSpeed);
	KvSetFloat(list, "rumble_strength", options->rumbleStrength);
	KvSetFloat(list, "controller_deadzone", options->controllerDeadzone);
	KvSetBool(list, "controller_swap_ok_cancel", options->controllerSwapOkCancel);

	KvList controls;
	KvListCreate(controls);
	SaveControls(controls);
	KvSetList(list, "controls", controls);

	KvList debugEntries;
	KvListCreate(debugEntries);
	SaveDebugEntrySettings(debugEntries);
	KvSetList(list, "debug_options", debugEntries);

	KvSetParamArray(list, "enabled_addons", SaveAddonSettings());

	KvSetBool(list, "fullscreen", options->fullscreen);
	KvSetBool(list, "vsync", options->vsync);
	KvSetBool(list, "prefer_wayland", options->preferWayland);
	KvSetBool(list, "limit_fps_when_unfocused", options->limitFpsWhenUnfocused);
	KvSetFloat(list, "fov", options->fov);
	KvSetInt(list, "max_fps", options->maxFps);
	KvSetByte(list, "preferred_gpu_type", options->preferredGpuType);

	const VideoPreset currentPreset = GetCurrentVideoPreset(options);
	if (currentPreset != VIDEO_PRESET_CUSTOM)
	{
		KvSetByte(list, "video_preset", currentPreset);
	} else
	{
		KvSetByte(list, "msaa", options->msaa);
		KvSetBool(list, "mipmaps", options->mipmaps);
		KvSetFloat(list, "lod_multiplier", options->lodMultiplier);
		KvSetByte(list, "anisotropy", options->anisotropy);
	}

	KvSetFloat(list, "music_volume", options->musicVolume);
	KvSetFloat(list, "sfx_volume", options->sfxVolume);
	KvSetFloat(list, "ui_volume", options->uiVolume);
	KvSetFloat(list, "master_volume", options->masterVolume);

	if (!WriteKvlFile(OPTIONS_FILE, list))
	{
		LogError("Failed to save options!\n");
	}

	KvListDestroy(controls);
	KvListDestroy(debugEntries);
}
