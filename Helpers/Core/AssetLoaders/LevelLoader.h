//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LEVELLOADER_H
#define GAME_LEVELLOADER_H

#include "../../../defines.h"

/**
 * Load a level from level bytecode
 * @param data Level bytecode
 * @param dataSize
 * @return Level struct
 */
Level *LoadLevel(const byte *data, size_t dataSize);

#endif //GAME_LEVELLOADER_H
