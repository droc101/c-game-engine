//
// Created by droc101 on 4/27/2024.
//

#ifndef GAME_DATAREADER_H
#define GAME_DATAREADER_H

#include <stddef.h>
#include <stdint.h>

/**
 * Reads a double from the given data at the given offset
 * @param data Data to read from
 * @param offset Offset to read from
 * @return The double read
 * @note Increments the offset by @c sizeof(double)
 */
double ReadDouble(const uint8_t *data, size_t *offset);

/**
 * Reads a uint32_t from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The uint32_t read
 * @note Increments the offset by @c sizeof(uint32_t)
 */
uint32_t ReadUint(const uint8_t *data, size_t *offset);

/**
 * Reads an int from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The int read
 * @note Increments the offset by @c sizeof(int)
 */
int ReadInt(const uint8_t *data, size_t *offset);

/**
 * Reads a float from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The float read
 * @note Increments the offset by @c sizeof(float)
 */
float ReadFloat(const uint8_t *data, size_t *offset);

/**
 * Reads a byte from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The byte read
 * @note Increments the offset by @c sizeof(uint8_t)
 */
uint8_t ReadByte(const uint8_t *data, size_t *offset);

/**
 * Reads a string of length @c len from the given data at the given offset into @c dest
 * @param data The data to read from
 * @param offset The offset to read from
 * @param dest The pointer to read the string into
 * @param len The length of the string to read
 * @note Increments the offset by @c len
 */
void ReadString(const uint8_t *data, size_t *offset, char *dest, size_t len);

/**
 * Reads a short from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The short read
 * @note Increments the offset by @c sizeof(short)
 */
short ReadShort(const uint8_t *data, size_t *offset);

/**
 * Reads arbitrary bytes from the given data at the given offset into dest
 * @param data The data to read from
 * @param offset The offset to read from
 * @param len The length of the data to read
 * @param dest The buffer to write the data into
 * @note It is up to the caller to prevent out of bounds access
 */
void ReadBytes(const uint8_t *data, size_t *offset, size_t len, void *dest);

/**
 * Reads a size_t from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The size_t read
 * @note Increments the offset by @c sizeof(size_t)
 */
size_t ReadSizeT(const uint8_t *data, size_t *offset);

/**
 * Reads a length and string from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @param totalBufferSize The total size of the data
 * @param outLength Where to write the length of the string read
 * @return A pointer (must be freed) of the string read
 */
char *ReadStringSafe(const uint8_t *data, size_t *offset, size_t totalBufferSize, size_t *outLength);

#endif //GAME_DATAREADER_H
