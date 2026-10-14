//
// Created by NBT22 on 9/10/26.
//

#ifndef GAME_MACROS_H
#define GAME_MACROS_H

/**
 * Get the number of elements in an array
 * @warning This macro only works with stack arrays! It will not work on any heap allocations.
 * @param arr The array to get the length of
 */
#define ArrayLength(arr) (sizeof(arr) / sizeof(*(arr)))

/**
 * Gets the size of a single field on a struct
 * @param Type The struct on which the field is found
 * @param member The field to get the size of
 */
#define SizeofMember(Type, member) (sizeof(((Type *)0)->member))

#endif //GAME_MACROS_H
