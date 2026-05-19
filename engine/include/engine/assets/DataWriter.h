//
// Created by droc101 on 5/19/26.
//

#ifndef GAME_DATAWRITER_H
#define GAME_DATAWRITER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct DataWriter DataWriter;
#define DATAWRITER_BUFFER_EXPANSION_SIZE 1024

DataWriter *CreateDataWriter();
void FreeDataWriter(DataWriter *writer);
const uint8_t *DataWriterGetBuffer(const DataWriter *writer);
size_t DataWriterGetBufferSize(const DataWriter *writer);
bool DataWriterIsEmpty(const DataWriter *writer);

void WriteBuffer(DataWriter *writer, const void *buffer, size_t elementSize, size_t numElements);
void WriteString(DataWriter *writer, const char* string);

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
