//
// Created by droc101 on 4/7/26.
//

#include <cglm/euler.h>
#include <cglm/quat.h>
#include <engine/helpers/MathEx.h>
#include <engine/physics/MapPhysics.h>
#include <engine/physics/Physics.h>
#include <engine/physics/PlayerPhysics.h>
#include <engine/structs/Actor.h>
#include <engine/structs/ActorDefinition.h>
#include <engine/structs/Camera.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Item.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Player.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/Logging.h>
#include <engine/subsystem/threads/LodThread.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <math.h>
#include <SDL3/SDL_gamepad.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static inline void UpdateCamera(GlobalState *state, const Vector2 cameraMotion)
{
	Camera *playerCamera = &state->map->player.playerCamera;
	Transform *transform = state->map->player.isFreecamActive ? &playerCamera->transform
															  : &state->map->player.transform;
	const float currentPitch = JPH_Quat_GetRotationAngle(&transform->rotation, &Vector3_AxisX) + GLM_PI_2f;
	JPH_Quat newYaw;
	JPH_Quat newPitch;
	JPH_Quat_Rotation(&Vector3_AxisY, cameraMotion.x, &newYaw);
	JPH_Quat_Rotation(&Vector3_AxisX, clamp(currentPitch + cameraMotion.y, 0, PIf) - currentPitch, &newPitch);
	JPH_Quat_Multiply(&newYaw, &transform->rotation, &transform->rotation);
	JPH_Quat_Multiply(&transform->rotation, &newPitch, &transform->rotation);
	JPH_Quat_Normalized(&transform->rotation, &transform->rotation);

	playerCamera->transform.position.x = transform->position.x;
	playerCamera->transform.position.z = transform->position.z;
	playerCamera->transform.rotation = transform->rotation;
	float yPos = transform->position.y;
	if (!state->map->player.isFreecamActive)
	{
		yPos += 0.25f;
	}
	playerCamera->transform.position.y = yPos;
	playerCamera->showPlayerModel = state->map->player.isFreecamActive;
}

void MapUpdate(GlobalState *state)
{
	if (!state->map)
	{
		return;
	}

	Vector2 cameraMotion = v2s(0);
	if (state->camera == &state->map->player.playerCamera)
	{
		cameraMotion = GetMouseRel(mainThreadInput);
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

void MapFixedUpdate(GlobalState *state, const double delta)
{
	if (!state->map)
	{
		return;
	}

	const bool allowMovement = state->camera == &state->map->player.playerCamera;

	float distanceTraveled = 0;
	MovePlayer(&state->map->player, &distanceTraveled, delta, allowMovement);

	// TODO: Why is controller rotation handed on the physics thread
	if (UseController() && allowMovement)
	{
		Vector2 cameraMotion = v2s(0);

		float cx = -GetAxis(physicsThreadInput, SDL_GAMEPAD_AXIS_RIGHTX);
		if (state->options.invertHorizontalCamera)
		{
			cx *= -1;
		}
		if (fabsf(cx) > STICK_DEADZONE)
		{
			cameraMotion.x = cx * state->options.cameraSpeed / 11.25f;
		}

		float cy = -GetAxis(physicsThreadInput, SDL_GAMEPAD_AXIS_RIGHTY);
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

	const float bobHeight = remap(distanceTraveled, 0, MOVE_SPEED / PHYSICS_TARGET_TPS, 0, 0.00175);
	state->map->player.viewBobbingHeight = 0.1f + (float)sin((double)state->physicsFrame / 7.0) * bobHeight;

	WaitForLodThreadToEnd();
	// WARNING: Any access to `state->level->actors` with ANY chance of modifying it MUST not happen before this!

	const float deltaTime = (float)delta / PHYSICS_TARGET_TPS;

	UpdatePlayer(&state->map->player, state->map->physicsSystem, deltaTime, allowMovement);

	JPH_CharacterVirtual_GetPosition(state->map->player.joltCharacter, &state->map->player.transform.position);

	for (size_t i = 0; i < state->map->actors.length; i++)
	{
		Actor *a = ListGetPointer(state->map->actors, i);
		a->definition->Update(a, delta);
	}

	// TODO proper UI for switching items
	if (allowMovement)
	{
		const float scroll = GetMouseWheel(physicsThreadInput).y;
		if (scroll > 0)
		{
			PreviousItem();
		} else if (scroll < 0)
		{
			NextItem();
		}
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
	SignalLodThreadCanStart();
}
