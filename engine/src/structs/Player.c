//
// Created by NBT22 on 7/31/25.
//

#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/physics/PlayerPhysics.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Player.h>
#include <joltc/joltc.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <SDL3/SDL_mutex.h>
#include <stddef.h>

void CreatePlayer(Player *player, JPH_PhysicsSystem *physicsSystem)
{
	player->transform.rotation = JPH_Quat_Identity;
	player->playerCamera.fov = GetState()->options.fov;
	player->playerCamera.nearZ = DEFAULT_NEAR_Z;
	player->playerCamera.farZ = DEFAULT_FAR_Z;
	player->playerCamera.transform.rotation = JPH_Quat_Identity;
	player->mutex = SDL_CreateMutex();

	CreatePlayerPhysics(player, physicsSystem);
}

void DestroyPlayer(Player *player)
{
	DestroyMutex(&player->mutex);
}
