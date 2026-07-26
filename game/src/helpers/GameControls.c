//
// Created by droc101 on 7/26/26.
//

#include "helpers/GameControls.h"
#include <engine/structs/ControlOptions.h>
#include <engine/structs/InputAction.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>

static const InputAction DEFAULT_SPAWN_TEST_ACTOR = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_UNBOUND,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_L,
			},
};
static const InputAction DEFAULT_SPAWN_CUBE = {
	.keyboardMouseBindType = IA_KEY,
	.controllerBindType = IA_UNBOUND,
	.keyboardMouseBind =
			{
				.keyBind = SDL_SCANCODE_C,
			},
};

InputAction spawnTestActor;
InputAction spawnCube;

void RegisterGameControls()
{
	ControlCategory *debugCat = GetControlCategory("debug");
	RegisterControl("spawn_test_actor",
					"Spawn Test Actor",
					&spawnTestActor,
					false,
					&DEFAULT_SPAWN_TEST_ACTOR,
					debugCat);
	RegisterControl("spawn_physbox", "Spawn Physbox", &spawnCube, false, &DEFAULT_SPAWN_CUBE, debugCat);
}
