//
// Created by droc101 on 10/27/24.
//

#ifndef GAME_OPTIONS_H
#define GAME_OPTIONS_H

#include <engine/structs/InputAction.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum OptionsMsaa OptionsMsaa;
typedef enum OptionsAnisotropy OptionsAnisotropy;

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

	InputAction moveForward;
	InputAction moveBackward;
	InputAction moveLeft;
	InputAction moveRight;
	InputAction sprint;
	InputAction sneak;
	InputAction jump;

	InputAction interact;
	InputAction primaryAttack;
	InputAction secondaryAttack;
	InputAction previousItem;
	InputAction nextItem;

	InputAction lookUp;
	InputAction lookDown;
	InputAction lookLeft;
	InputAction lookRight;

	InputAction debugMenu;
	InputAction noclip;
	InputAction freecam;
	InputAction benchmark;

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
	/// Field of view
	float fov;
	/// Anisotropy level
	OptionsAnisotropy anisotropy;
	/// The FPS cap, or 0 for no cap
	uint16_t maxFps;

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

#pragma region Default Input Actions

extern const InputAction DEFAULT_MOVE_FORWARD;
extern const InputAction DEFAULT_MOVE_BACKWARD;
extern const InputAction DEFAULT_MOVE_LEFT;
extern const InputAction DEFAULT_MOVE_RIGHT;
extern const InputAction DEFAULT_SPRINT;
extern const InputAction DEFAULT_SNEAK;
extern const InputAction DEFAULT_JUMP;

extern const InputAction DEFAULT_INTERACT;
extern const InputAction DEFAULT_PRIMARY_ATTACK;
extern const InputAction DEFAULT_SECONDARY_ATTACK;
extern const InputAction DEFAULT_PREVIOUS_ITEM;
extern const InputAction DEFAULT_NEXT_ITEM;

extern const InputAction DEFAULT_LOOK_UP;
extern const InputAction DEFAULT_LOOK_DOWN;
extern const InputAction DEFAULT_LOOK_LEFT;
extern const InputAction DEFAULT_LOOK_RIGHT;

extern const InputAction DEFAULT_DEBUG_MENU;
extern const InputAction DEFAULT_NOCLIP;
extern const InputAction DEFAULT_FREECAM;
extern const InputAction DEFAULT_BENCHMARK;

#pragma endregion

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
