//
// Created by droc101 on 4/27/2024.
//

#ifndef GAME_DATAREADER_H
#define GAME_DATAREADER_H

#include "../../defines.h"

// The "A" functions don't increment the offset, they just read from the given offset

/**
 * Reads a double from the given data at the given offset
 * @param data Data to read from
 * @param offset Offset to read from
 * @return The double read
 * @note Increments the offset by 8
 */
double ReadDouble(const byte *data, size_t *offset);

/**
 * Reads a double from the given data at the given offset, but doesn't increment the offset
 * @param data Data to read from
 * @param offset Offset to read from
 * @return The double read
 */
double ReadDoubleA(const byte *data, size_t offset);

/**
 * Reads a uint from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The uint read
 * @note Increments the offset by 4
 */
uint ReadUint(const byte *data, size_t *offset);

/**
 * Reads an int from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The int read
 * @note Increments the offset by 4
 */
int ReadInt(const byte *data, size_t *offset);

/**
 * Reads a uint from the given data at the given offset, but doesn't increment the offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The uint read
 */
uint ReadUintA(const byte *data, size_t offset);

/**
 * Reads a float from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The float read
 * @note Increments the offset by 4
 */
float ReadFloat(const byte *data, size_t *offset);

/**
 * Reads a byte from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The byte read
 * @note Increments the offset by 1
 */
byte ReadByte(const byte *data, size_t *offset);

/**
 * Reads a string of length @c len from the given data at the given offset into @c dest
 * @param data The data to read from
 * @param offset The offset to read from
 * @param dest The pointer to read the string into
 * @param len The length of the string to read
 * @note Increments the offset by @c len
 */
void ReadString(const byte *data, size_t *offset, char *dest, size_t len);

/**
 * Reads a short from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The short read
 * @note Increments the offset by 2
 */
short ReadShort(const byte *data, size_t *offset);

/**
 * Reads arbitrary bytes from the given data at the given offset into dest
 * @param data The data to read from
 * @param offset The offset to read from
 * @param len The length of the data to read
 * @param dest The buffer to write the data into
 * @note It is up to the caller to prevent out of bounds access
 */
void ReadBytes(const byte *data, size_t *offset, size_t len, void *dest);

size_t ReadSizeT(const byte *data, size_t *offset);

#endif //GAME_DATAREADER_H
