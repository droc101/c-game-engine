//
// Created by droc101 on 10/27/24.
//

#include <engine/assets/KvlFile.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Options.h>
#include <engine/subsystem/Logging.h>
#include <stdbool.h>
#include <stddef.h>

#define OPTIONS_FILE "options.kvl"

void DefaultOptions(Options *options)
{
	options->enableDiscordRpc = true;
	options->renderer = RENDERER_OPENGL; // TODO: RENDERER_VULKAN;
	options->musicVolume = 1.0f;
	options->sfxVolume = 1.0f;
	options->uiVolume = 1.0f;
	options->masterVolume = 1.0f;
	options->fullscreen = false;
	options->lodMultiplier = 1.0f;
	options->cameraSpeed = 1;
	options->controllerMode = false;
	options->msaa = MSAA_4X;
	options->mipmaps = true;
	options->rumbleStrength = 1.0f;
	options->invertHorizontalCamera = false;
	options->invertVerticalCamera = false;
	options->controllerSwapOkCancel = false;
	options->preferWayland = true;
	options->fov = 90.0f;
	options->anisotropy = ANISOTROPY_16X;
	options->maxFps = 0;
#ifdef BUILDSTYLE_DEBUG
	options->vsync = false;
	options->limitFpsWhenUnfocused = false;
#else
	options->vsync = true;
	options->limitFpsWhenUnfocused = true;
#endif
}

bool ValidateOptions(const Options *options)
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

	if (options->renderer >= RENDERER_MAX)
	{
		return false;
	}
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


	if (options->musicVolume < 0 || options->musicVolume > 1)
	{
		return false;
	}
	if (options->sfxVolume < 0 || options->sfxVolume > 1)
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
	KvList list;
	if (ReadKvlFile(OPTIONS_FILE, list))
	{
		options->enableDiscordRpc = KvGetBool(list, "enable_discord_rpc", true);
		options->controllerMode = KvGetBool(list, "controller_mode", false);
		options->cameraSpeed = KvGetFloat(list, "camera_speed", 1.0f);
		options->rumbleStrength = KvGetFloat(list, "rumble_strength", 1.0f);
		options->invertHorizontalCamera = KvGetBool(list, "invert_horizontal_camera", false);
		options->invertVerticalCamera = KvGetBool(list, "invert_vertical_camera", false);
		options->controllerSwapOkCancel = KvGetBool(list, "controller_swap_ok_cancel", false);

		options->renderer = KvGetByte(list, "renderer", RENDERER_OPENGL);
		options->fullscreen = KvGetBool(list, "fullscreen", false);
		options->vsync = KvGetBool(list, "vsync", true);
		options->msaa = KvGetByte(list, "msaa", MSAA_4X);
		options->mipmaps = KvGetBool(list, "mipmaps", true);
		options->preferWayland = KvGetBool(list, "prefer_wayland", true);
		options->limitFpsWhenUnfocused = KvGetBool(list, "limit_fps_when_unfocused", true);
		options->lodMultiplier = KvGetFloat(list, "lod_multiplier", 1.0f);
		options->fov = KvGetFloat(list, "fov", 90.0f);
		options->anisotropy = KvGetByte(list, "anisotropy", ANISOTROPY_16X);
		options->maxFps = KvGetInt(list, "max_fps", 0);

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

	KvSetBool(list, "controller_mode", options->controllerMode);
	KvSetFloat(list, "camera_speed", options->cameraSpeed);
	KvSetFloat(list, "rumble_strength", options->rumbleStrength);
	KvSetBool(list, "invert_horizontal_camera", options->invertHorizontalCamera);
	KvSetBool(list, "invert_vertical_camera", options->invertVerticalCamera);
	KvSetBool(list, "controller_swap_ok_cancel", options->controllerSwapOkCancel);

	KvSetByte(list, "renderer", options->renderer);
	KvSetBool(list, "fullscreen", options->fullscreen);
	KvSetBool(list, "vsync", options->vsync);
	KvSetByte(list, "msaa", options->msaa);
	KvSetBool(list, "mipmaps", options->mipmaps);
	KvSetBool(list, "prefer_wayland", options->preferWayland);
	KvSetBool(list, "limit_fps_when_unfocused", options->limitFpsWhenUnfocused);
	KvSetFloat(list, "lod_multiplier", options->lodMultiplier);
	KvSetFloat(list, "fov", options->fov);
	KvSetByte(list, "anisotropy", options->anisotropy);
	KvSetInt(list, "max_fps", options->maxFps);

	KvSetFloat(list, "music_volume", options->musicVolume);
	KvSetFloat(list, "sfx_volume", options->sfxVolume);
	KvSetFloat(list, "ui_volume", options->uiVolume);
	KvSetFloat(list, "master_volume", options->masterVolume);

	if (!WriteKvlFile(OPTIONS_FILE, list))
	{
		LogError("Failed to save options!");
	}
}
