//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_MAPLOADER_H
#define GAME_MAPLOADER_H

#include <engine/structs/Map.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Load a map asset
 * @param path The asset path
 * @return The loaded map
 */
Map *LoadMap(const char *path);

#endif //GAME_MAPLOADER_H
