//
// Created by NBT22 on 7/31/25.
//

#include <engine/graphics/RenderingHelpers.h>
#include <engine/physics/PlayerPhysics.h>
#include <engine/structs/GlobalState.h>
#include <engine/structs/Light.h>
#include <engine/structs/Map.h>
#include <engine/structs/Player.h>
#include <joltc/Math/Quat.h>
#include <joltc/Math/Transform.h>
#include <stddef.h>

void CreatePlayer(Map *map)
{
	map->player.transform.rotation = JPH_Quat_Identity;
	map->player.playerCamera.fov = GetState()->options.fov;
	map->player.playerCamera.nearPlane = DEFAULT_NEAR_PLANE;
	map->player.playerCamera.farPlane = DEFAULT_FAR_PLANE;
	map->player.playerCamera.transform.rotation = JPH_Quat_Identity;
	map->player.playerCamera.recomputeCachedData = true;

	map->player.flashlight.parent = playerBodyId;
	map->player.flashlight.light.type = LIGHT_TYPE_SPOT;
	map->player.flashlight.light.color[0] = 1;
	map->player.flashlight.light.color[1] = 1;
	map->player.flashlight.light.color[2] = 1;
	map->player.flashlight.light.brightness = 1;
	map->player.flashlight.light.quadraticAttenuation = 1;
	map->player.flashlight.light.attenuationMultiplier = 100;
	map->player.flashlight.light.brightAngle = 15;
	map->player.flashlight.light.fadingAngle = 25;

	CreatePlayerPhysics(map);
}
