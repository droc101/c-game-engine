//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_MAPLOADER_H
#define GAME_MAPLOADER_H

#include <engine/structs/Map.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Load a map asset
 * @param map The map to load into
 * @param mapData The asset to load from
 * @return Whether the map was loaded sucessfully. If this is false, do not use @c map
 */
bool LoadMap(Map *map, Asset *mapData);

#endif //GAME_MAPLOADER_H
