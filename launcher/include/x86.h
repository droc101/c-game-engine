//
// Created by droc101 on 11/14/25.
//

#ifndef GAME_X86_H
#define GAME_X86_H

#include <stdint.h>

/**
 * Get the CLI argument requesting an ABI level override
 * @return 0 for no override, otherwise the requested ABI level
 */
uint8_t GetX86AbiArgument(int argc, const char *argv[]);

/**
 * Get this system's ABI level
 */
uint8_t GetX86AbiLevel();

/**
 * Get the library name for a given ABI level
 * @param abiLevel The ABI level, 1 to 4
 */
const char *GetX86LibraryName(uint8_t abiLevel);

#endif //GAME_X86_H
