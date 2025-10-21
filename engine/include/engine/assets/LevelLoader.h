//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LEVELLOADER_H
#define GAME_LEVELLOADER_H

#include <stddef.h>
#include <stdint.h>
#include <engine/structs/Level.h>

/**
 * Load a level from level bytecode
 * @param data Level bytecode
 * @param dataSize
 * @return Level struct
 */
Level *LoadLevel(const uint8_t *data, size_t dataSize);

#endif //GAME_LEVELLOADER_H
