//
// Created by droc101 on 4/22/2024.
//

#include "gameState/MainState.h"
#include <cglm/euler.h>
#include <cglm/quat.h>
#include <engine/assets/AssetReader.h>
#include <engine/debug/DPrint.h>
#include <engine/Engine.h>
#include <engine/graphics/Drawing.h>
#include <engine/graphics/RenderingHelpers.h>
#include <engine/physics/MapPhysics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Color.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Item.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Player.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/SoundSystem.h>
#include <joltc/joltc.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "actor/Physbox.h"
#include "actor/TestActor.h"
#include "gameState/PauseState.h"

static const char *spawnActorOnce = NULL;
static const char *spawnActorEveryTick = NULL;

void MainStateUpdate(GlobalState *state)
{
	if (IsKeyJustPressed(mainThreadInput, SDL_SCANCODE_ESCAPE) ||
		IsButtonJustPressed(mainThreadInput, SDL_GAMEPAD_BUTTON_START) ||
		!IsWindowFocused())
	{
		(void)PlaySound(SOUND("sfx/popup"), SOUND_CATEGORY_UI);
		SetGameState(&PauseState);
		return;
	}

	MapUpdate(state);
}

void MainStateFixedUpdate(GlobalState *state, const double delta)
{
	if (IsKeyJustPressed(physicsThreadInput, SDL_SCANCODE_L))
	{
		if (spawnActorEveryTick == NULL || strcmp(spawnActorEveryTick, TEST_ACTOR_NAME) != 0)
		{
			spawnActorEveryTick = TEST_ACTOR_NAME;
		} else
		{
			spawnActorEveryTick = NULL;
		}
	}
	if (IsKeyJustPressed(physicsThreadInput, SDL_SCANCODE_C))
	{
		spawnActorOnce = PHYSBOX_ACTOR_NAME;
	}

	if (spawnActorOnce)
	{
		Actor *actor = CreateActor(&state->map->player.transform,
								   spawnActorOnce,
								   NULL,
								   JPH_PhysicsSystem_GetBodyInterface(state->map->physicsSystem));
		AddActor(actor);
		spawnActorOnce = NULL;
	}
	if (spawnActorEveryTick)
	{
		Actor *actor = CreateActor(&state->map->player.transform,
								   spawnActorEveryTick,
								   NULL,
								   JPH_PhysicsSystem_GetBodyInterface(state->map->physicsSystem));
		AddActor(actor);
	}

	MapFixedUpdate(state, delta);
}

void MainStateRender(GlobalState *state)
{
	// warp the mouse to the center of the screen
	const Vector2 realWndSize = ActualWindowSize();
	SDL_WarpMouseInWindow(GetGameWindow(), realWndSize.x / 2, realWndSize.y / 2);

	RenderMap(state->map, state->camera);
	RenderHUD();

#ifdef BUILDSTYLE_DEBUG
	DPrintF("Engine " ENGINE_VERSION, false, COLOR_WHITE);
#endif
	DPrintPlayer(&state->map->player);

	DPrintF("Actors: %d", false, COLOR_WHITE, state->map->actors.length);
}

void MainStateSet()
{
	GetState()->rpcState = IN_GAME;
}

const GameState MainState = {
	.UpdateGame = MainStateUpdate,
	.RenderGame = MainStateRender,
	.FixedUpdateGame = MainStateFixedUpdate,
	.Destroy = NULL,
	.Set = MainStateSet,
	.enableRelativeMouseMode = true,
};
