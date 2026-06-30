//
// Created by droc101 on 5/19/26.
//

#include <engine/assets/DataWriter.h>
#include <engine/subsystem/Error.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/// The internal buffer of a DataWriter will always be a multiple of this size
#define DATAWRITER_BUFFER_BLOCK_SIZE 1024

struct DataWriter
{
	uint8_t *buffer;
	size_t usedSpace;
	size_t bufferSize;
};

DataWriter *CreateDataWriter()
{
	DataWriter *writer = malloc(sizeof(DataWriter));
	CheckAlloc(writer);
	writer->buffer = malloc(DATAWRITER_BUFFER_BLOCK_SIZE);
	writer->usedSpace = 0;
	writer->bufferSize = DATAWRITER_BUFFER_BLOCK_SIZE;
	return writer;
}

void FreeDataWriter(DataWriter *writer)
{
	free(writer->buffer);
	free(writer);
}

const uint8_t *DataWriterGetBuffer(const DataWriter *writer)
{
	return writer->buffer;
}

size_t DataWriterGetBufferSize(const DataWriter *writer)
{
	return writer->usedSpace;
}

bool DataWriterIsEmpty(const DataWriter *writer)
{
	return writer->usedSpace == 0;
}

/**
 * Ensure that a DataWriter has enough space to write a given number of bytes
 * @param neededSpace The amount of free memory needed in bytes
 */
static void DataWriterEnsureSpace(DataWriter *writer, const size_t neededSpace)
{
	if (writer->bufferSize - writer->usedSpace < neededSpace)
	{
		const size_t blocksNeeded = (size_t)ceilf((float)neededSpace / DATAWRITER_BUFFER_BLOCK_SIZE) + 1;
		const size_t newSize = writer->bufferSize + (blocksNeeded * DATAWRITER_BUFFER_BLOCK_SIZE);
		writer->buffer = realloc(writer->buffer, newSize);
		CheckAlloc(writer->buffer);
		writer->bufferSize = newSize;
	}
}

void WriteBuffer(DataWriter *writer, const void *buffer, const size_t elementSize, const size_t numElements)
{
	const size_t size = elementSize * numElements;
	DataWriterEnsureSpace(writer, size);
	memcpy(writer->buffer + writer->usedSpace, buffer, size);
	writer->usedSpace += size;
}

void WriteString(DataWriter *writer, const char *string)
{
	const size_t len = strlen(string);
	WriteSizeT(writer, len + 1);
	WriteBuffer(writer, string, 1, len);
	WriteUint8(writer, 0); // null terminator
}

#define DefineWriteFunction(T, name) \
	DeclareWriteFunction(T, name) \
	{ \
		DataWriterEnsureSpace(writer, sizeof(T)); \
		memcpy(writer->buffer + writer->usedSpace, &data, sizeof(T)); \
		writer->usedSpace += sizeof(T); \
	}

DefineWriteFunction(int8_t, WriteInt8);
DefineWriteFunction(uint8_t, WriteUint8);

DefineWriteFunction(int16_t, WriteInt16);
DefineWriteFunction(uint16_t, WriteUint16);

DefineWriteFunction(uint32_t, WriteUint32);
DefineWriteFunction(int32_t, WriteInt32);

DefineWriteFunction(uint64_t, WriteUint64);
DefineWriteFunction(int64_t, WriteInt64);
DefineWriteFunction(size_t, WriteSizeT);

DefineWriteFunction(double, WriteDouble);
DefineWriteFunction(float, WriteFloat);
