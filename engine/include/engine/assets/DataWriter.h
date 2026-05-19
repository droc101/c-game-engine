//
// Created by droc101 on 5/19/26.
//

#ifndef GAME_DATAWRITER_H
#define GAME_DATAWRITER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct DataWriter DataWriter;

/**
 * Create a DataWriter
 */
DataWriter *CreateDataWriter();

/**
 * Free a DataWriter
 */
void FreeDataWriter(DataWriter *writer);

/**
 * Get the buffer of a DataWriter (the written data)
 */
const uint8_t *DataWriterGetBuffer(const DataWriter *writer);

/**
 * Get the size of a DataWriter's buffer
 * @note The actual buffer may be larger than this size, but the remaining space is not used
 */
size_t DataWriterGetBufferSize(const DataWriter *writer);

/**
 * Check if a DataWriter is empty (no data written)
 */
bool DataWriterIsEmpty(const DataWriter *writer);

/**
 * Write a buffer or array to a DataWriter
 * @param writer The DataWriter to write to
 * @param buffer The buffer/array to write
 * @param elementSize The size of each element in the buffer
 * @param numElements The number of elements in the buffer
 */
void WriteBuffer(DataWriter *writer, const void *buffer, size_t elementSize, size_t numElements);

/**
 * Write a string to a DataWriter
 */
void WriteString(DataWriter *writer, const char *string);

#define DeclareWriteFunction(T, name) void name(DataWriter *writer, const T data)

DeclareWriteFunction(int8_t, WriteInt8);
DeclareWriteFunction(uint8_t, WriteUint8);

DeclareWriteFunction(int16_t, WriteInt16);
DeclareWriteFunction(uint16_t, WriteUint16);

DeclareWriteFunction(uint32_t, WriteUint32);
DeclareWriteFunction(int32_t, WriteInt32);

DeclareWriteFunction(uint64_t, WriteUint64);
DeclareWriteFunction(int64_t, WriteInt64);
DeclareWriteFunction(size_t, WriteSizeT);

DeclareWriteFunction(double, WriteDouble);
DeclareWriteFunction(float, WriteFloat);

#endif //GAME_DATAWRITER_H
