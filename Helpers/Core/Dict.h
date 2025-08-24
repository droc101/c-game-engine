//
// Created by NBT22 on 8/13/25.
//

#ifndef GAME_DICT_H
#define GAME_DICT_H

#include <m-dict.h>

#define DICT_DEF(name, keyType, ...) \
	_Pragma("GCC diagnostic push"); \
	_Pragma("GCC diagnostic ignored \"-Wunused-function\""); \
	M_DICT_DEF2_AS(name, name, name##_iterator, name##_pair, keyType, __VA_ARGS__); \
	_Pragma("GCC diagnostic pop");

#endif //GAME_DICT_H
