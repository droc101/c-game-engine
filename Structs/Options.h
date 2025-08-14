//
// Created by droc101 on 10/27/24.
//

#ifndef GAME_OPTIONS_H
#define GAME_OPTIONS_H

#include <stdbool.h>
#include <stdint.h>
#include "../Helpers/Graphics/RenderingHelpers.h"

typedef enum OptionsMsaa OptionsMsaa;

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

struct Options
{
	/// Checksum of the options struct (helps prevent corruption)
	uint16_t checksum;

	/* Controls */

	/// Whether the game is in controller mode
	bool controllerMode;
	/// The look speed (it affects controller speed too)
	float cameraSpeed;
	/// The strength of the rumble
	float rumbleStrength;
	/// Whether to invert the camera Y axis
	bool invertHorizontalCamera;
	bool invertVerticalCamera;
	/// Whether to swap the controller A and B buttons
	bool controllerSwapOkCancel;

	/* Video */

	/// The renderer to use
	Renderer renderer;
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

	/* Audio */

	/// The volume of the music
	float musicVolume;
	/// The volume of the sound effects
	float sfxVolume;
	/// The master volume
	float masterVolume;
} __attribute__((packed)); // This is packed because it is saved to disk

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
