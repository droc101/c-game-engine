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
#include <engine/structs/ControlOptions.h>
#include <engine/structs/GameState.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/InputAction.h>
#include <engine/structs/Item.h>
#include <engine/structs/List.h>
#include <engine/structs/Map.h>
#include <engine/structs/Player.h>
#include <engine/structs/Vector2.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Input.h>
#include <engine/subsystem/Logging.h>
#include <joltc/enums.h>
#include <joltc/joltc.h>
#include <joltc/Math/Transform.h>
#include <joltc/Math/Vector3.h>
#include <math.h>
#include <SDL3/SDL_mutex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

void MapUpdate(GlobalState *state, const double delta)
{
	if (!state->map)
	{
		return;
	}

	UpdatePlayerCamera(state, delta);

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

	SDL_LockMutex(state->map->player.mutex);
	Vector3_Add(&state->map->player.transform.position,
				&state->map->player.deltaPosition,
				&state->map->player.transform.position);
	Vector3 newPosition;
	JPH_CharacterVirtual_GetPosition(state->map->player.joltCharacter, &newPosition);
	Vector3_Subtract(&newPosition, &state->map->player.transform.position, &state->map->player.deltaPosition);
	SDL_UnlockMutex(state->map->player.mutex);

	const bool allowMovement = state->camera == &state->map->player.playerCamera;

	MovePlayer(&state->map->player, delta, allowMovement);

	const float deltaTime = (float)(delta / PHYSICS_TARGET_TPS);

	UpdatePlayer(&state->map->player, state->map->physicsSystem, deltaTime, allowMovement);

	const float distanceTraveled = sqrtf(state->map->player.deltaPosition.x * state->map->player.deltaPosition.x +
										 state->map->player.deltaPosition.z * state->map->player.deltaPosition.z);
	const float bobHeight = remap(distanceTraveled, 0, MOVE_SPEED / PHYSICS_TARGET_TPS, 0, 0.1);
	state->map->player.viewBobbingHeight = 0.1f +
										   sinf((float)(fmod((double)state->physicsFrame / 7.0, 2 * PI))) * bobHeight;

	for (size_t i = 0; i < state->map->actors.length; i++)
	{
		Actor *a = ListGetPointer(state->map->actors, i);
		a->definition->Update(a, delta);
	}

	ProcessIOQueue(state->map);

	// TODO proper UI for switching items
	if (allowMovement)
	{
		if (IsInputActionJustPressed(physicsThreadInput, &nextItem))
		{
			NextItem();
		} else if (IsInputActionJustPressed(physicsThreadInput, &previousItem))
		{
			PreviousItem();
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
}
