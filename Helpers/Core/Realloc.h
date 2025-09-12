//
// Created by NBT22 on 9/2/25.
//

#ifndef GAME_REALLOC_H
#define GAME_REALLOC_H

#include <stddef.h>

/**
 * Reallocates memory for an array of arrayLength elements of size bytes each.
 * @param ptr Pointer to the memory block to be reallocated.
 * @param arrayLength Number of elements.
 * @param elementSize Size of each element.
 * @return Pointer to the reallocated memory block.
 */
void *GameReallocArray(void *ptr, size_t arrayLength, size_t elementSize);

/**
 *
 * @param ptr The pointer to reallocate
 * @param oldSize
 * @param newSize
 * @return
 */
void *Recalloc(void *ptr, size_t oldSize, size_t newSize);

#endif //GAME_REALLOC_H
