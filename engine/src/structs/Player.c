//
// Created by NBT22 on 7/31/25.
//

#include <engine/physics/PlayerPhysics.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Map.h>
#include <engine/structs/Player.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <stddef.h>

void CreatePlayer(Map *map)
{
	map->player.transform.rotation = JPH_Quat_Identity;
	map->player.playerCamera.fov = GetState()->options.fov;
	map->player.playerCamera.transform.rotation = JPH_Quat_Identity;

	CreatePlayerPhysics(map);
}
