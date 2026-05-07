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
#include <joltc/Math/Transform.h>
#include <math.h>
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

	const bool allowMovement = state->camera == &state->map->player.playerCamera;

	MovePlayer(&state->map->player, delta, allowMovement);

	WaitForLodThreadToEnd();
	// WARNING: Any access to `state->level->actors` with ANY chance of modifying it MUST not happen before this!

	const float deltaTime = (float)delta / PHYSICS_TARGET_TPS;

	UpdatePlayer(&state->map->player, state->map->physicsSystem, deltaTime, allowMovement);

	Vector2 oldPosition = v2(state->map->player.transform.position.x, state->map->player.transform.position.z);
	JPH_CharacterVirtual_GetPosition(state->map->player.joltCharacter, &state->map->player.transform.position);

	const float distanceTraveled = Vector2Distance(oldPosition,
												   v2(state->map->player.transform.position.x,
													  state->map->player.transform.position.z));
	const float bobHeight = remap(distanceTraveled, 0, MOVE_SPEED / PHYSICS_TARGET_TPS, 0, 0.1);
	state->map->player.viewBobbingHeight = 0.1f + (float)sin((double)state->physicsFrame / 7.0) * bobHeight;

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
