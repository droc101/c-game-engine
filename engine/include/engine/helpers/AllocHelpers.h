//
// Created by NBT22 on 9/2/25.
//

/**
 * @file AllocHelpers.h
 * Memory reallocation functions
 */

#ifndef GAME_REALLOC_H
#define GAME_REALLOC_H

#include <stddef.h>

void *AllocArray(size_t arrayLength, size_t elementSize);

/**
 * Reallocates memory for an array of arrayLength elements of size bytes each.
 * @param ptr Pointer to the memory block to be reallocated.
 * @param arrayLength Number of elements.
 * @param elementSize Size of each element.
 * @return Pointer to the reallocated memory block.
 */
void *ReallocArray(void *ptr, size_t arrayLength, size_t elementSize);

/**
 * Reallocate a pointer with a new size while keeping the contents
 * @param ptr The pointer to reallocate
 * @param oldSize The size of the current (soon to be old) data
 * @param newSize The new size of the data
 * @return A pointer to the new data
 * @warning This will crash the engine on allocation failure. It does *not* return NULL.
 */
void *Recalloc(void *ptr, size_t oldSize, size_t newSize);

/**
 * Make a zero-initalized heap allocation aligned to AVX requirements
 * @param size The size of the allocation
 * @return Pointer to allocated memory, or @c NULL on failure
 */
void *AvxAlignedCalloc(size_t size);

/**
 * Free AVX-aligned memory (such as but not limited to memory allocated by @c AvxAlignedCalloc)
 * @param data The memory to free
 */
void AvxAlignedFree(void *data);

#endif //GAME_REALLOC_H
