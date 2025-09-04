//
// Created by NBT22 on 8/13/25.
//

#ifndef GAME_DICT_H
#define GAME_DICT_H

#include <m-core.h>
#include <m-dict.h>
#include <stdio.h>

#define DEFINE_DICT(name, keyType, ...) \
	_Pragma("GCC diagnostic push"); \
	_Pragma("GCC diagnostic ignored \"-Wunused-function\""); \
	M_DICT_DEF2_AS(name, name, name##_iterator, name##_pair, keyType, __VA_ARGS__); \
	_Pragma("GCC diagnostic pop");

#define STR_COPY(str, value) \
	(str) = malloc(strlen((char*)value) + 1); \
	((char *)(str))[strlen((char*)value)] = '\0'; \
	strncpy((char*)(str), (char*)(value), strlen((char*)(value)));
#define STR_FREE(str) \
	free((char*)(str)); \
	(str) = NULL;

#define STR_OPLIST \
	(INIT(M_INIT_DEFAULT), \
	 INIT_SET(STR_COPY), \
	 SET(STR_COPY), \
	 CLEAR(STR_FREE), \
	 HASH(m_core_cstr_hash), \
	 EQUAL(M_CSTR_EQUAL), \
	 CMP(strcmp), \
	 TYPE(const char *))

#endif //GAME_DICT_H
