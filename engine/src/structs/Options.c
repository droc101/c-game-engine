//
// Created by droc101 on 10/27/24.
//

#include <engine/assets/KvlFile.h>
#include <engine/structs/InputAction.h>
#include <engine/structs/KVList.h>
#include <engine/structs/Options.h>
#include <engine/subsystem/Logging.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define OPTIONS_FILE "options.kvl"

#pragma region Default Input Actions

const InputAction DEFAULT_MOVE_FORWARD = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_W,
			},
	.controllerBind = {.axisBind = LEFT_STICK_UP},
};
const InputAction DEFAULT_MOVE_BACKWARD = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_S,
			},
	.controllerBind = {.axisBind = LEFT_STICK_DOWN},
};
const InputAction DEFAULT_MOVE_LEFT = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_A,
			},
	.controllerBind = {.axisBind = LEFT_STICK_LEFT},
};
const InputAction DEFAULT_MOVE_RIGHT = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_D,
			},
	.controllerBind = {.axisBind = LEFT_STICK_RIGHT},
};
const InputAction DEFAULT_SPRINT = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_LSHIFT,
			},
	.controllerBind =
			{
				.axisBind = LEFT_TRIGGER,
			},
};
const InputAction DEFAULT_SNEAK = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_LCTRL,
			},
	.controllerBind = {.axisBind = RIGHT_TRIGGER},
};
const InputAction DEFAULT_JUMP = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_BUTTON,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_SPACE,
			},
	.controllerBind = {.buttonBind = SDL_GAMEPAD_BUTTON_EAST},
};

const InputAction DEFAULT_INTERACT = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_BUTTON,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_E,
			},
	.controllerBind =
			{
				.buttonBind = SDL_GAMEPAD_BUTTON_SOUTH,
			},
};
const InputAction DEFAULT_PRIMARY_ATTACK = {
	.keyboardMouseBindType = IA_MOUSE_BUTTON,
	.controllerBindType = IA_CONTROLLER_BUTTON,
	.keyboardMouseBind =
			{
				.mouseButtonBind = SDL_BUTTON_LEFT,
			},
	.controllerBind =
			{
				.buttonBind = SDL_GAMEPAD_BUTTON_WEST,
			},
};
const InputAction DEFAULT_SECONDARY_ATTACK = {
	.keyboardMouseBindType = IA_MOUSE_BUTTON,
	.controllerBindType = IA_CONTROLLER_BUTTON,
	.keyboardMouseBind =
			{
				.mouseButtonBind = SDL_BUTTON_RIGHT,
			},
	.controllerBind =
			{
				.buttonBind = SDL_GAMEPAD_BUTTON_NORTH,
			},
};
const InputAction DEFAULT_PREVIOUS_ITEM = {
	.keyboardMouseBindType = IA_MOUSE_WHEEL,
	.controllerBindType = IA_CONTROLLER_BUTTON,
	.keyboardMouseBind =
			{
				.mouseWheelBind = MOUSE_WHEEL_UP,
			},
	.controllerBind =
			{
				.buttonBind = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
			},
};
const InputAction DEFAULT_NEXT_ITEM = {
	.keyboardMouseBindType = IA_MOUSE_WHEEL,
	.controllerBindType = IA_CONTROLLER_BUTTON,
	.keyboardMouseBind =
			{
				.mouseWheelBind = MOUSE_WHEEL_DOWN,
			},
	.controllerBind =
			{
				.buttonBind = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
			},
};

const InputAction DEFAULT_LOOK_UP = {
	.keyboardMouseBindType = IA_UNBOUND,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.controllerBind = {.axisBind = RIGHT_STICK_UP},
};
const InputAction DEFAULT_LOOK_DOWN = {
	.keyboardMouseBindType = IA_UNBOUND,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.controllerBind = {.axisBind = RIGHT_STICK_DOWN},
};
const InputAction DEFAULT_LOOK_LEFT = {
	.keyboardMouseBindType = IA_UNBOUND,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.controllerBind = {.axisBind = RIGHT_STICK_LEFT},
};
const InputAction DEFAULT_LOOK_RIGHT = {
	.keyboardMouseBindType = IA_UNBOUND,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.controllerBind = {.axisBind = RIGHT_STICK_RIGHT},
};

const InputAction DEFAULT_DEBUG_MENU = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_UNBOUND,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_F4,
			},
};
const InputAction DEFAULT_NOCLIP = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_UNBOUND,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_V,
			},
};
const InputAction DEFAULT_FREECAM = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_UNBOUND,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_F8,
			},
};
const InputAction DEFAULT_BENCHMARK = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_UNBOUND,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_F10,
			},
};

#pragma endregion

static void DefaultOptions(Options *options)
{
	options->enableDiscordRpc = true;
	options->musicVolume = 1.0f;
	options->sfxVolume = 1.0f;
	options->uiVolume = 1.0f;
	options->masterVolume = 1.0f;
	options->fullscreen = false;
	options->lodMultiplier = 1.0f;
	options->cameraSpeed = 1;
	options->msaa = MSAA_4X;
	options->mipmaps = true;
	options->rumbleStrength = 1.0f;
	options->controllerDeadzone = 0.1f;
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

	memcpy(&options->moveForward, &DEFAULT_MOVE_FORWARD, sizeof(InputAction));
	memcpy(&options->moveBackward, &DEFAULT_MOVE_BACKWARD, sizeof(InputAction));
	memcpy(&options->moveLeft, &DEFAULT_MOVE_LEFT, sizeof(InputAction));
	memcpy(&options->moveRight, &DEFAULT_MOVE_RIGHT, sizeof(InputAction));
	memcpy(&options->sprint, &DEFAULT_SPRINT, sizeof(InputAction));
	memcpy(&options->sneak, &DEFAULT_SNEAK, sizeof(InputAction));
	memcpy(&options->jump, &DEFAULT_JUMP, sizeof(InputAction));

	memcpy(&options->interact, &DEFAULT_INTERACT, sizeof(InputAction));
	memcpy(&options->primaryAttack, &DEFAULT_PRIMARY_ATTACK, sizeof(InputAction));
	memcpy(&options->secondaryAttack, &DEFAULT_SECONDARY_ATTACK, sizeof(InputAction));
	memcpy(&options->previousItem, &DEFAULT_PREVIOUS_ITEM, sizeof(InputAction));
	memcpy(&options->nextItem, &DEFAULT_NEXT_ITEM, sizeof(InputAction));

	memcpy(&options->lookUp, &DEFAULT_LOOK_UP, sizeof(InputAction));
	memcpy(&options->lookDown, &DEFAULT_LOOK_DOWN, sizeof(InputAction));
	memcpy(&options->lookLeft, &DEFAULT_LOOK_LEFT, sizeof(InputAction));
	memcpy(&options->lookRight, &DEFAULT_LOOK_RIGHT, sizeof(InputAction));

	memcpy(&options->debugMenu, &DEFAULT_DEBUG_MENU, sizeof(InputAction));
	memcpy(&options->noclip, &DEFAULT_NOCLIP, sizeof(InputAction));
	memcpy(&options->freecam, &DEFAULT_FREECAM, sizeof(InputAction));
	memcpy(&options->benchmark, &DEFAULT_BENCHMARK, sizeof(InputAction));
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
	KvList list;
	if (ReadKvlFile(OPTIONS_FILE, list))
	{
		options->enableDiscordRpc = KvGetBool(list, "enable_discord_rpc", true);
		options->cameraSpeed = KvGetFloat(list, "camera_speed", 1.0f);
		options->rumbleStrength = KvGetFloat(list, "rumble_strength", 1.0f);
		options->controllerDeadzone = KvGetFloat(list, "controller_deadzone", 0.1f);
		options->controllerSwapOkCancel = KvGetBool(list, "controller_swap_ok_cancel", false);

		LoadInputAction("move_forward", list, DEFAULT_MOVE_FORWARD, &options->moveForward);
		LoadInputAction("move_backward", list, DEFAULT_MOVE_BACKWARD, &options->moveBackward);
		LoadInputAction("move_left", list, DEFAULT_MOVE_LEFT, &options->moveLeft);
		LoadInputAction("move_right", list, DEFAULT_MOVE_RIGHT, &options->moveRight);
		LoadInputAction("sprint", list, DEFAULT_SPRINT, &options->sprint);
		LoadInputAction("sneak", list, DEFAULT_SNEAK, &options->sneak);
		LoadInputAction("jump", list, DEFAULT_JUMP, &options->jump);


		LoadInputAction("interact", list, DEFAULT_INTERACT, &options->interact);
		LoadInputAction("primary_attack", list, DEFAULT_PRIMARY_ATTACK, &options->primaryAttack);
		LoadInputAction("secondary_attack", list, DEFAULT_SECONDARY_ATTACK, &options->secondaryAttack);
		LoadInputAction("previous_item", list, DEFAULT_PREVIOUS_ITEM, &options->previousItem);
		LoadInputAction("next_item", list, DEFAULT_NEXT_ITEM, &options->nextItem);

		LoadInputAction("look_up", list, DEFAULT_LOOK_UP, &options->lookUp);
		LoadInputAction("look_down", list, DEFAULT_LOOK_DOWN, &options->lookDown);
		LoadInputAction("look_left", list, DEFAULT_LOOK_LEFT, &options->lookLeft);
		LoadInputAction("look_right", list, DEFAULT_LOOK_RIGHT, &options->lookRight);

		LoadInputAction("toggle_debug_menu", list, DEFAULT_DEBUG_MENU, &options->debugMenu);
		LoadInputAction("toggle_noclip", list, DEFAULT_NOCLIP, &options->noclip);
		LoadInputAction("toggle_freecam", list, DEFAULT_FREECAM, &options->freecam);
		LoadInputAction("toggle_benchmark", list, DEFAULT_BENCHMARK, &options->benchmark);


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

	KvSetFloat(list, "camera_speed", options->cameraSpeed);
	KvSetFloat(list, "rumble_strength", options->rumbleStrength);
	KvSetFloat(list, "controller_deadzone", options->controllerDeadzone);
	KvSetBool(list, "controller_swap_ok_cancel", options->controllerSwapOkCancel);

	SaveInputAction("move_forward", list, &options->moveForward);
	SaveInputAction("move_backward", list, &options->moveBackward);
	SaveInputAction("move_left", list, &options->moveLeft);
	SaveInputAction("move_right", list, &options->moveRight);
	SaveInputAction("sprint", list, &options->sprint);
	SaveInputAction("sneak", list, &options->sneak);
	SaveInputAction("jump", list, &options->jump);
	SaveInputAction("interact", list, &options->interact);
	SaveInputAction("primary_attack", list, &options->primaryAttack);
	SaveInputAction("secondary_attack", list, &options->secondaryAttack);
	SaveInputAction("previous_item", list, &options->previousItem);
	SaveInputAction("next_item", list, &options->nextItem);
	SaveInputAction("look_up", list, &options->lookUp);
	SaveInputAction("look_down", list, &options->lookDown);
	SaveInputAction("look_left", list, &options->lookLeft);
	SaveInputAction("look_right", list, &options->lookRight);
	SaveInputAction("toggle_debug_menu", list, &options->debugMenu);
	SaveInputAction("toggle_noclip", list, &options->noclip);
	SaveInputAction("toggle_freecam", list, &options->freecam);
	SaveInputAction("toggle_benchmark", list, &options->benchmark);

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
		LogError("Failed to save options!\n");
	}
}
