//
// Created by droc101 on 4/27/2024.
//

#ifndef GAME_DATAREADER_H
#define GAME_DATAREADER_H

#include <engine/structs/Asset.h>
#include <stddef.h>
#include <stdint.h>

typedef struct DataReader DataReader;

/**
 * Create a DataReader
 * @param data The data buffer to read from
 * @param bufferSize The size of the data buffer
 * @param offset The initial offset into the buffer
 * @return The DataReader
 */
DataReader *CreateDataReader(void *data, size_t bufferSize, size_t offset);

/**
 * Create a DataReader from an Asset
 * @param asset The Asset to read from
 * @return The DataReader
 */
DataReader *CreateDataReaderFromAsset(Asset *asset);

/**
 * Destroy a DataReader
 */
void DestroyDataReader(DataReader *reader);

#define DeclareReadFunction(T, name) T name(DataReader *reader)

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
 * @param reader The DataReader to read from
 * @param readSize The length of the data to read
 * @param dest The buffer to write the data into
 * @note It is up to the caller to prevent out of bounds access
 */
void ReadBuffer(DataReader *reader, size_t readSize, void *dest);

/**
 * Reads a length and string from the given data at the given offset
 * @param reader The DataReader to read to
 * @param outLength Where to write the length of the string read
 * @return A pointer (must be freed) of the string read
 */
char *ReadStringSafe(DataReader *reader, size_t *outLength);

/**
 * Seek ahead in a DataReader without performing any reads
 * @param reader The DataReader to seek
 * @param bytes The number of bytes to seek ahead
 */
void Seek(DataReader *reader, size_t bytes);

/**
 * Get the current offset a DataReader is at
 */
size_t DataReaderGetOffset(const DataReader *reader);

/**
 * Calculate a 16-bit checksum of a given data buffer
 * @param buffer The data to checksum
 * @param bufferSize The size of the data
 * @return 16-bit checksum of the given data
 */
uint16_t Checksum(const uint8_t *buffer, size_t bufferSize);

#endif //GAME_DATAREADER_H
