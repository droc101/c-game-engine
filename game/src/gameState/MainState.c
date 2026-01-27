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
#include <engine/helpers/MathEx.h>
#include <engine/physics/Physics.h>
#include <engine/physics/PlayerPhysics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Item.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Player.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Discord.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/SoundSystem.h>
#include <engine/subsystem/threads/LodThread.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <math.h>
#include <SDL_gamecontroller.h>
#include <SDL_mouse.h>
#include <SDL_scancode.h>
#include <SDL_stdinc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "actor/Physbox.h"
#include "actor/TestActor.h"
#include "gameState/PauseState.h"

static bool lodThreadInitDone = false;

static inline void UpdateCamera(GlobalState *state, const Vector2 cameraMotion)
{
	Transform *transform = state->map->player.isFreecamActive ? &state->camera->transform
															  : &state->map->player.transform;
	const float currentPitch = JPH_Quat_GetRotationAngle(&state->map->player.transform.rotation, &Vector3_AxisX) +
							   GLM_PI_2f;
	JPH_Quat newYaw;
	JPH_Quat newPitch;
	JPH_Quat_Rotation(&Vector3_AxisY, cameraMotion.x, &newYaw);
	JPH_Quat_Rotation(&Vector3_AxisX, clamp(currentPitch + cameraMotion.y, 0, PIf) - currentPitch, &newPitch);
	JPH_Quat_Multiply(&newYaw, &transform->rotation, &transform->rotation);
	JPH_Quat_Multiply(&transform->rotation, &newPitch, &transform->rotation);
	JPH_Quat_Normalized(&transform->rotation, &transform->rotation);

	state->camera->transform.position.x = transform->position.x;
	state->camera->transform.position.y = transform->position.y; // + state->camera->yOffset;
	state->camera->transform.position.z = transform->position.z;
	state->camera->transform.rotation = transform->rotation;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void MainStateUpdate(GlobalState *state)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_START))
	{
		(void)PlaySoundEffect(SOUND("sfx/popup"), 0, 1, NULL, NULL);
		PauseStateSet();
		return;
	}

	Vector2 cameraMotion = GetMouseRel();
	cameraMotion.x *= -state->options.cameraSpeed / 120.0f;
	cameraMotion.y *= -state->options.cameraSpeed / 120.0f;
	if (state->options.invertHorizontalCamera)
	{
		cameraMotion.x *= -1;
	}
	if (state->options.invertVerticalCamera)
	{
		cameraMotion.y *= -1;
	}

	UpdateCamera(state, cameraMotion);

	Item *item = GetItem();
	if (item)
	{
		item->definition->Update(item, state);
	}

	if (state->saveData->coins > 9999)
	{
		state->saveData->coins = 9999;
	}
	if (state->saveData->blueCoins > 5)
	{
		state->saveData->blueCoins = 5;
	}
}

void MainStateFixedUpdate(GlobalState *state, const double delta)
{
	float distanceTraveled = 0;
	MovePlayer(&state->map->player, &distanceTraveled, delta);

	// TODO: Why is controller rotation handed on the physics thread
	if (UseController())
	{
		Vector2 cameraMotion = v2s(0);

		float cx = -GetAxis(SDL_CONTROLLER_AXIS_RIGHTX);
		if (state->options.invertHorizontalCamera)
		{
			cx *= -1;
		}
		if (fabsf(cx) > STICK_DEADZONE)
		{
			cameraMotion.x = cx * state->options.cameraSpeed / 11.25f;
		}

		float cy = -GetAxis(SDL_CONTROLLER_AXIS_RIGHTY);
		if (state->options.invertVerticalCamera)
		{
			cy *= -1;
		}
		if (fabsf(cy) > STICK_DEADZONE)
		{
			cameraMotion.y = cy * state->options.cameraSpeed / 11.25f;
		}

		UpdateCamera(state, cameraMotion);
	}

	Item *item = GetItem();
	if (item)
	{
		item->definition->FixedUpdate(item, state, delta);
	}

	const float bobHeight = remap(distanceTraveled, 0, MOVE_SPEED / PHYSICS_TARGET_TPS, 0, 0.00175);
	state->camera->yOffset = 0.1f + (float)sin((double)state->physicsFrame / 7.0) * bobHeight;

	if (WaitForLodThreadToEnd() != 0)
	{
		Error("Failed to wait for LOD thread end semaphore!");
	}
	// WARNING: Any access to `state->level->actors` with ANY chance of modifying it MUST not happen before this!

	const float deltaTime = (float)delta / PHYSICS_TARGET_TPS;

	UpdatePlayer(&state->map->player, state->map->physicsSystem, deltaTime);

	JPH_CharacterVirtual_GetPosition(state->map->player.joltCharacter, &state->map->player.transform.position);

	for (size_t i = 0; i < state->map->actors.length; i++)
	{
		Actor *a = ListGetPointer(state->map->actors, i);
		a->definition->Update(a, delta);
	}

	if (IsKeyJustPressedPhys(SDL_SCANCODE_L))
	{
		Actor *leaf = CreateActor(&state->map->player.transform,
								  TEST_ACTOR_NAME,
								  NULL,
								  JPH_PhysicsSystem_GetBodyInterface(state->map->physicsSystem));
		AddActor(leaf);
	}

	if (IsKeyJustPressedPhys(SDL_SCANCODE_C))
	{
		Actor *leaf = CreateActor(&state->map->player.transform,
								  PHYSBOX_ACTOR_NAME,
								  NULL,
								  JPH_PhysicsSystem_GetBodyInterface(state->map->physicsSystem));
		AddActor(leaf);
	}

	// TODO proper UI for switching items
	if (IsKeyJustPressedPhys(SDL_SCANCODE_Q))
	{
		PreviousItem();
	} else if (IsKeyJustPressedPhys(SDL_SCANCODE_E))
	{
		NextItem();
	}

	const JPH_PhysicsUpdateError result = JPH_PhysicsSystem_Update(state->map->physicsSystem,
																   deltaTime,
																   2,
																   state->jobSystem);
	if (result != JPH_PhysicsUpdateError_None)
	{
		LogError("Failed to update Jolt physics system with error %d\n", result);
		Error("Failed to update physics!");
	}
	GetState()->map->physicsTick++;

	// WARNING: Any access to `state->level->actors` with ANY chance of modifying it MUST not happen after this!
	if (SignalLodThreadCanStart() != 0)
	{
		Error("Failed to signal LOD thread start semaphore!");
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void MainStateRender(GlobalState *state)
{
	// warp the mouse to the center of the screen
	const Vector2 realWndSize = ActualWindowSize();
	SDL_WarpMouseInWindow(GetGameWindow(), (int)realWndSize.x / 2, (int)realWndSize.y / 2);

	RenderMap(state->map, state->camera);
	RenderHUD();

	Item *item = GetItem();
	if (item)
	{
		item->definition->RenderHud(item);
	}

#ifdef BUILDSTYLE_DEBUG
	DPrintF("Engine " ENGINE_VERSION, COLOR_WHITE, false);
#endif
	DPrintPlayer(&state->map->player);

	DPrintF("Actors: %d", COLOR_WHITE, false, state->map->actors.length);
}

void MainStateSet()
{
	GetState()->rpcState = IN_GAME;
	if (!lodThreadInitDone)
	{
		LodThreadInit();
		lodThreadInitDone = true;
	}
	SetStateCallbacks(MainStateUpdate, MainStateFixedUpdate, GAME_STATE_MAIN, MainStateRender, SDL_TRUE);
}
