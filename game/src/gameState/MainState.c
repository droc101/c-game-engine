//
// Created by droc101 on 4/22/2024.
//

#include "gameState/MainState.h"
#include <cglm/euler.h>
#include <cglm/quat.h>
#include <engine/assets/AssetReader.h>
#include <engine/debug/DPrint.h>
#include <engine/graphics/Drawing.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/Physics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Color.h>
#include <engine/structs/GlobalState.h>
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
#include <joltc/Math/Vector3.h>
#include <math.h>
#include <SDL_gamecontroller.h>
#include <SDL_mouse.h>
#include <SDL_scancode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "actor/TestActor.h"
#include "engine/Engine.h"

#include "engine/physics/PlayerPhysics.h"
#include "gameState/PauseState.h"

static bool lodThreadInitDone = false;

static inline void RotateCamera(const Vector2 cameraMotion)
{
	const float currentPitch = JPH_Quat_GetRotationAngle(&GetState()->level->player.transform.rotation,
														 &Vector3_AxisX) +
							   GLM_PI_2f;
	JPH_Quat newYaw;
	JPH_Quat newPitch;
	JPH_Quat_Rotation(&Vector3_AxisY, cameraMotion.x, &newYaw);
	JPH_Quat_Rotation(&Vector3_AxisX, clamp(currentPitch + cameraMotion.y, 0, PIf) - currentPitch, &newPitch);
	JPH_Quat_Multiply(&newYaw,
					  &GetState()->level->player.transform.rotation,
					  &GetState()->level->player.transform.rotation);
	JPH_Quat_Multiply(&GetState()->level->player.transform.rotation,
					  &newPitch,
					  &GetState()->level->player.transform.rotation);
	JPH_Quat_Normalized(&GetState()->level->player.transform.rotation, &GetState()->level->player.transform.rotation);
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

	RotateCamera(cameraMotion);

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
	MovePlayer(&state->level->player, &distanceTraveled);

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

		RotateCamera(cameraMotion);
	}

	const float bobHeight = remap(distanceTraveled, 0, MOVE_SPEED / PHYSICS_TARGET_TPS, 0, 0.00175);
	state->camera->yOffset = 0.1f + (float)sin((double)state->physicsFrame / 7.0) * bobHeight;

	if (WaitForLodThreadToEnd() != 0)
	{
		Error("Failed to wait for LOD thread end semaphore!");
	}
	// WARNING: Any access to `state->level->actors` with ANY chance of modifying it MUST not happen before this!

	const float deltaTime = (float)delta / PHYSICS_TARGET_TPS;

	UpdatePlayer(&state->level->player, state->level->physicsSystem, deltaTime);

	JPH_CharacterVirtual_GetPosition(state->level->player.joltCharacter, &state->level->player.transform.position);

	for (size_t i = 0; i < state->level->actors.length; i++)
	{
		Actor *a = ListGetPointer(state->level->actors, i);
		a->definition->Update(a, delta);
	}

	if (IsKeyJustPressedPhys(SDL_SCANCODE_L))
	{
		Actor *leaf = CreateActor(&state->level->player.transform,
								  TEST_ACTOR_NAME,
								  NULL,
								  JPH_PhysicsSystem_GetBodyInterface(state->level->physicsSystem));
		AddActor(leaf);
	}

	const JPH_PhysicsUpdateError result = JPH_PhysicsSystem_Update(state->level->physicsSystem,
																   deltaTime,
																   2,
																   state->jobSystem);
	if (result != JPH_PhysicsUpdateError_None)
	{
		LogError("Failed to update Jolt physics system with error %d\n", result);
		Error("Failed to update physics!");
	}

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

	const Map *level = state->level;

	RenderLevel(level, state->camera);
	RenderHUD();

#ifdef BUILDSTYLE_DEBUG
	DPrintF("Engine " ENGINE_VERSION, COLOR_WHITE, false);
#endif
	DPrintPlayer(&level->player);

	DPrintF("Actors: %d", COLOR_WHITE, false, level->actors.length);
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
