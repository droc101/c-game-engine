//
// Created by droc101 on 4/27/2024.
//

#ifndef GAME_DATAREADER_H
#define GAME_DATAREADER_H

#include <stddef.h>
#include <stdint.h>

#define DeclareReadFunction(T, name) T name(const uint8_t *data, size_t *offset, const size_t totalBufferSize)

DeclareReadFunction(int8_t, ReadInt8);
DeclareReadFunction(uint8_t, ReadUint8);

DeclareReadFunction(int16_t, ReadInt16);
DeclareReadFunction(uint16_t, ReadUint16);

DeclareReadFunction(uint32_t, ReadUint32);
DeclareReadFunction(int32_t, ReadInt32);

DeclareReadFunction(uint64_t, ReadUint64);
DeclareReadFunction(int64_t, ReadInt64);
DeclareReadFunction(size_t, ReadSizeT);

DeclareReadFunction(double, ReadDouble);
DeclareReadFunction(float, ReadFloat);

/**
 * Reads arbitrary bytes from the given data at the given offset into dest
 * @param data The data to read from
 * @param offset The offset to read from
 * @param dataSize The total size of the data to read from
 * @param readSize The length of the data to read
 * @param dest The buffer to write the data into
 * @note It is up to the caller to prevent out of bounds access
 */
void ReadBuffer(const uint8_t *data, size_t *offset, size_t dataSize, size_t readSize, void *dest);

/**
 * Reads a length and string from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @param totalBufferSize The total size of the data
 * @param outLength Where to write the length of the string read
 * @return A pointer (must be freed) of the string read
 */
char *ReadStringSafe(const uint8_t *data, size_t *offset, size_t totalBufferSize, size_t *outLength);

/**
 * Calculate a 16-bit checksum of a given data buffer
 * @param buffer The data to checksum
 * @param bufferSize The size of the data
 * @return 16-bit checksum of the given data
 */
uint16_t Checksum(const uint8_t *buffer, size_t bufferSize);

#endif //GAME_DATAREADER_H
