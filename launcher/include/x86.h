//
// Created by droc101 on 11/14/25.
//

#ifndef GAME_X86_H
#define GAME_X86_H

#include <stdint.h>

uint8_t GetX86AbiArgument(int argc, const char *argv[]);

uint8_t GetX86AbiLevel();

const char *GetX86LibraryName(uint8_t abiLevel);

#endif //GAME_X86_H
