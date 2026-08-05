//
// Created by droc101 on 10/27/24.
//

#ifndef GAME_OPTIONS_H
#define GAME_OPTIONS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum OptionsMsaa OptionsMsaa;
typedef enum OptionsAnisotropy OptionsAnisotropy;
typedef enum OptionsGpuType OptionsGpuType;
typedef enum OptionsShadowMapResolution OptionsShadowMapResolution;

typedef struct Options Options;

/**
 * Used the check the MSAA level setting
 */
enum OptionsMsaa
{
	MSAA_NONE = 0,
	MSAA_2X = 1,
	MSAA_4X = 2,
	MSAA_8X = 3
};

enum OptionsAnisotropy
{
	ANISOTROPY_NONE = 0,
	ANISOTROPY_2X = 1,
	ANISOTROPY_4X = 2,
	ANISOTROPY_8X = 3,
	ANISOTROPY_16X = 4
};

enum OptionsGpuType
{
	GPU_TYPE_INTEGRATED,
	GPU_TYPE_DEDICATED,
	GPU_TYPE_SOFTWARE,
};

enum OptionsShadowMapResolution
{
	SHADOW_MAP_RESOLUTION_DISABLED = 0,
	SHADOW_MAP_RESOLUTION_128 = 1,
	SHADOW_MAP_RESOLUTION_256 = 2,
	SHADOW_MAP_RESOLUTION_512 = 3,
	SHADOW_MAP_RESOLUTION_1024 = 4,
	SHADOW_MAP_RESOLUTION_2048 = 5,
	SHADOW_MAP_RESOLUTION_4096 = 6,
	SHADOW_MAP_RESOLUTION_8192 = 7,
	SHADOW_MAP_RESOLUTION_16384 = 8,
};

struct Options
{
	bool enableDiscordRpc;

	/* Controls */
	/// The look speed (it affects controller speed too)
	float cameraSpeed;
	/// The strength of the rumble
	float rumbleStrength;
	/// Whether to invert the camera X axis
	bool invertHorizontalCamera;
	/// Whether to invert the camera Y axis
	bool invertVerticalCamera;
	/// Controller axis deadzone
	float controllerDeadzone;
	/// Whether to swap the controller A and B buttons
	bool controllerSwapOkCancel;

	/* Video */

	/// Whether the game is fullscreen
	bool fullscreen;
	/// Whether vsync is enabled
	bool vsync;
	/// The MSAA level
	OptionsMsaa msaa;
	/// Whether to use mipmaps
	bool mipmaps;
	/// Whether to prefer Wayland over X11
	bool preferWayland;
	/// Whether to drop to 30 fps when the window is not focused
	bool limitFpsWhenUnfocused;
	/// The LOD distance multiplier
	float lodMultiplier;
	/// What the quality of the realtime shadow maps should be
	OptionsShadowMapResolution shadowMapQuality;
	/// Field of view
	float fov;
	/// Anisotropy level
	OptionsAnisotropy anisotropy;
	/// The FPS cap, or 0 for no cap
	uint16_t maxFps;
	OptionsGpuType preferredGpuType;

	/* Audio */

	/// The volume of the music
	float musicVolume;
	/// The volume of the sound effects
	float sfxVolume;
	/// The volume of UI sounds
	float uiVolume;
	/// The master volume
	float masterVolume;
};

/**
 * Read options from disk, or load defaults if no options file is found or is invalid
 * @param options The options to load into
 */
void LoadOptions(Options *options);

/**
 * Save options to disk
 * @param options The options to save
 */
void SaveOptions(Options *options);

#endif //GAME_OPTIONS_H
