//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LEVELLOADER_H
#define GAME_LEVELLOADER_H

#include <engine/structs/Map.h>
#include <stddef.h>
#include <stdint.h>

Map *LoadMap(const char *path);

#endif //GAME_LEVELLOADER_H
