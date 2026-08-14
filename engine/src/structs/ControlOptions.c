//
// Created by droc101 on 7/26/26.
//

#include <engine/structs/ControlOptions.h>
#include <engine/structs/InputAction.h>
#include <engine/structs/KVList.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#pragma region Default Input Actions

static const InputAction DEFAULT_MOVE_FORWARD = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_W,
			},
	.controllerBind = {.axisBind = LEFT_STICK_UP},
};
static const InputAction DEFAULT_MOVE_BACKWARD = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_S,
			},
	.controllerBind = {.axisBind = LEFT_STICK_DOWN},
};
static const InputAction DEFAULT_MOVE_LEFT = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_A,
			},
	.controllerBind = {.axisBind = LEFT_STICK_LEFT},
};
static const InputAction DEFAULT_MOVE_RIGHT = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_D,
			},
	.controllerBind = {.axisBind = LEFT_STICK_RIGHT},
};
static const InputAction DEFAULT_SPRINT = {
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
static const InputAction DEFAULT_SNEAK = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_LCTRL,
			},
	.controllerBind = {.axisBind = RIGHT_TRIGGER},
};
static const InputAction DEFAULT_JUMP = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_CONTROLLER_BUTTON,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_SPACE,
			},
	.controllerBind = {.buttonBind = SDL_GAMEPAD_BUTTON_EAST},
};

static const InputAction DEFAULT_INTERACT = {
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
static const InputAction DEFAULT_PRIMARY_ATTACK = {
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
static const InputAction DEFAULT_SECONDARY_ATTACK = {
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
static const InputAction DEFAULT_PREVIOUS_ITEM = {
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
static const InputAction DEFAULT_NEXT_ITEM = {
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

static const InputAction DEFAULT_LOOK_UP = {
	.keyboardMouseBindType = IA_UNBOUND,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.controllerBind = {.axisBind = RIGHT_STICK_UP},
};
static const InputAction DEFAULT_LOOK_DOWN = {
	.keyboardMouseBindType = IA_UNBOUND,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.controllerBind = {.axisBind = RIGHT_STICK_DOWN},
};
static const InputAction DEFAULT_LOOK_LEFT = {
	.keyboardMouseBindType = IA_UNBOUND,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.controllerBind = {.axisBind = RIGHT_STICK_LEFT},
};
static const InputAction DEFAULT_LOOK_RIGHT = {
	.keyboardMouseBindType = IA_UNBOUND,
	.controllerBindType = IA_CONTROLLER_AXIS,
	.controllerBind = {.axisBind = RIGHT_STICK_RIGHT},
};

static const InputAction DEFAULT_DEBUG_MENU = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_UNBOUND,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_F4,
			},
};
static const InputAction DEFAULT_NOCLIP = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_UNBOUND,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_V,
			},
};
static const InputAction DEFAULT_FREECAM = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_UNBOUND,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_F8,
			},
};
static const InputAction DEFAULT_BENCHMARK = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_UNBOUND,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_F10,
			},
};
static const InputAction DEFAULT_RELOAD_SHADERS = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_UNBOUND,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_F5,
			},
};
static const InputAction DEFAULT_TOGGLE_FULLSCREEN = {
	.keyboardMouseBindType = IA_KEY,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_F11,
			},
	.controllerBindType = IA_UNBOUND,
};

#pragma endregion

#pragma region Engine Input Actions

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
InputAction reloadShaders;

InputAction fullscreen;

#pragma endregion

List controlCategories;

void RegisterControl(char *key,
					 char *displayName,
					 InputAction *action,
					 bool allowAxisBind,
					 const InputAction *defaultAction,
					 ControlCategory *category)
{
	ControlOption *option = malloc(sizeof(ControlOption));
	CheckAlloc(option);
	strncpy(option->key, key, sizeof(option->key));
	strncpy(option->displayName, displayName, sizeof(option->displayName));
	option->action = action;
	option->allowAxisBind = allowAxisBind;
	option->defaultAction = defaultAction;
	ListAdd(category->controlOptions, option);
}

ControlCategory *GetControlCategory(const char *key)
{
	for (size_t i = 0; i < controlCategories.length; i++)
	{
		ControlCategory *c = ListGetPointer(controlCategories, i);
		if (strncmp(c->key, key, sizeof(c->key)) == 0)
		{
			return c;
		}
	}
	return NULL;
}

ControlCategory *RegisterControlCategory(char *displayName, char *key)
{
	ControlCategory *existing = GetControlCategory(key);
	if (existing)
	{
		return existing;
	}
	ControlCategory *category = malloc(sizeof(ControlCategory));
	CheckAlloc(category);
	strncpy(category->categoryName, displayName, sizeof(category->categoryName));
	strncpy(category->key, key, sizeof(category->key));
	ListInit(category->controlOptions, LIST_POINTER);
	ListAdd(controlCategories, category);
	return category;
}

void RegisterControls(const ControlRegisterFunction RegisterGameControls)
{
	ListInit(controlCategories, LIST_POINTER);

	ControlCategory *movementCat = RegisterControlCategory("Movement", "movement");
	RegisterControl("move_forward", "Move Forward", &moveForward, true, &DEFAULT_MOVE_FORWARD, movementCat);
	RegisterControl("move_backward", "Move Backward", &moveBackward, true, &DEFAULT_MOVE_BACKWARD, movementCat);
	RegisterControl("move_left", "Move Left", &moveLeft, true, &DEFAULT_MOVE_LEFT, movementCat);
	RegisterControl("move_right", "Move Right", &moveRight, true, &DEFAULT_MOVE_RIGHT, movementCat);
	RegisterControl("sprint", "Sprint", &sprint, true, &DEFAULT_SPRINT, movementCat);
	RegisterControl("sneak", "Sneak", &sneak, true, &DEFAULT_SNEAK, movementCat);
	RegisterControl("jump", "Jump", &jump, false, &DEFAULT_JUMP, movementCat);

	ControlCategory *interactionCat = RegisterControlCategory("Interaction", "interaction");
	RegisterControl("interact", "Interact", &interact, false, &DEFAULT_INTERACT, interactionCat);
	RegisterControl("primary_attack", "Primary Attack", &primaryAttack, false, &DEFAULT_PRIMARY_ATTACK, interactionCat);
	RegisterControl("secondary_attack",
					"Secondary Attack",
					&secondaryAttack,
					false,
					&DEFAULT_SECONDARY_ATTACK,
					interactionCat);
	RegisterControl("previous_item", "Previous Item", &previousItem, true, &DEFAULT_PREVIOUS_ITEM, interactionCat);
	RegisterControl("next_item", "Next Item", &nextItem, true, &DEFAULT_NEXT_ITEM, interactionCat);

	ControlCategory *cameraCat = RegisterControlCategory("Camera", "camera");
	RegisterControl("look_up", "Look Up", &lookUp, true, &DEFAULT_LOOK_UP, cameraCat);
	RegisterControl("look_down", "Look Down", &lookDown, true, &DEFAULT_LOOK_DOWN, cameraCat);
	RegisterControl("look_left", "Look Left", &lookLeft, true, &DEFAULT_LOOK_LEFT, cameraCat);
	RegisterControl("look_right", "Look Right", &lookRight, true, &DEFAULT_LOOK_RIGHT, cameraCat);

	ControlCategory *miscCat = RegisterControlCategory("Miscellaneous", "miscellaneous");
	RegisterControl("toggle_fullscreen", "Toggle Fullscreen", &fullscreen, false, &DEFAULT_TOGGLE_FULLSCREEN, miscCat);

	ControlCategory *debugCat = RegisterControlCategory("Debug", "debug");
	RegisterControl("toggle_debug_menu", "Toggle Debug Menu", &debugMenu, false, &DEFAULT_DEBUG_MENU, debugCat);
	RegisterControl("toggle_noclip", "Toggle Noclip", &noclip, false, &DEFAULT_NOCLIP, debugCat);
	RegisterControl("toggle_freecam", "Toggle Freecam", &freecam, false, &DEFAULT_FREECAM, debugCat);
	RegisterControl("toggle_benchmark", "Start/Stop Benchmark", &benchmark, false, &DEFAULT_BENCHMARK, debugCat);
	RegisterControl("reload_shaders", "Reload Shaders", &reloadShaders, false, &DEFAULT_RELOAD_SHADERS, debugCat);

	if (RegisterGameControls)
	{
		RegisterGameControls();
	}
}

void DefaultControls()
{
	for (size_t i = 0; i < controlCategories.length; i++)
	{
		ControlCategory *cat = ListGetPointer(controlCategories, i);
		for (size_t j = 0; j < cat->controlOptions.length; j++)
		{
			ControlOption *opt = ListGetPointer(cat->controlOptions, j);
			memcpy(opt->action, opt->defaultAction, sizeof(InputAction));
		}
	}
}

void LoadControls(KvList from)
{
	for (size_t i = 0; i < controlCategories.length; i++)
	{
		ControlCategory *cat = ListGetPointer(controlCategories, i);
		for (size_t j = 0; j < cat->controlOptions.length; j++)
		{
			const ControlOption *opt = ListGetPointer(cat->controlOptions, j);
			LoadInputAction(opt->key, from, *opt->defaultAction, opt->action);
		}
	}
}

void SaveControls(KvList to)
{
	for (size_t i = 0; i < controlCategories.length; i++)
	{
		ControlCategory *cat = ListGetPointer(controlCategories, i);
		for (size_t j = 0; j < cat->controlOptions.length; j++)
		{
			const ControlOption *opt = ListGetPointer(cat->controlOptions, j);
			SaveInputAction(opt->key, to, opt->action);
		}
	}
}

void DestroyControls()
{
	for (size_t i = 0; i < controlCategories.length; i++)
	{
		ControlCategory *cat = ListGetPointer(controlCategories, i);
		ListAndContentsFree(cat->controlOptions);
	}
	ListAndContentsFree(controlCategories);
}
