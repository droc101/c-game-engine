//
// Created by NBT22 on 7/31/25.
//

#include <engine/graphics/RenderingHelpers.h>
#include <engine/helpers/PlatformHelpers.h>
#include <engine/physics/PlayerPhysics.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Light.h>
#include <engine/structs/Map.h>
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
	player->playerCamera.nearPlane = DEFAULT_NEAR_PLANE;
	player->playerCamera.farPlane = DEFAULT_FAR_PLANE;
	player->playerCamera.transform.rotation = JPH_Quat_Identity;
	player->playerCamera.recomputeCachedData = true;
	player->mutex = SDL_CreateMutex();

	player->flashlight.parent = playerBodyId;
	player->flashlight.light.type = LIGHT_TYPE_SPOT;
	player->flashlight.light.color.r = 1;
	player->flashlight.light.color.g = 1;
	player->flashlight.light.color.b = 1;
	player->flashlight.light.color.a = 1;
	player->flashlight.light.brightness = 5;
	player->flashlight.light.constantAttenuation = 2.5f;
	player->flashlight.light.linearAttenuation = 1;
	player->flashlight.light.quadraticAttenuation = 1;
	player->flashlight.light.attenuationMultiplier = 25;
	player->flashlight.light.brightAngle = 15;
	player->flashlight.light.fadingAngle = 25;

	CreatePlayerPhysics(player, physicsSystem);
}

void DestroyPlayer(Player *player)
{
	DestroyMutex(&player->mutex);
}
