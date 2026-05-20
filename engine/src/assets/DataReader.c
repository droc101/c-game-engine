//
// Created by droc101 on 4/27/2024.
//

#include <assert.h>
#include <engine/assets/DataReader.h>
#include <engine/structs/Asset.h>
#include <engine/subsystem/Error.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct DataReader
{
	uint8_t *data;
	size_t offset;
	size_t totalBufferSize;
};

DataReader *CreateDataReader(void *data, const size_t bufferSize, const size_t offset)
{
	DataReader *reader = malloc(sizeof(DataReader));
	CheckAlloc(reader);
	reader->data = data;
	reader->totalBufferSize = bufferSize;
	reader->offset = offset;
	return reader;
}

DataReader *CreateDataReaderFromAsset(Asset *asset)
{
	return CreateDataReader(asset->data, asset->size, 0);
}

void DestroyDataReader(DataReader *reader)
{
	free(reader);
}

#define DefineReadFunction(T, name) \
	DeclareReadFunction(T, name) \
	{ \
		if (reader->offset + sizeof(T) > reader->totalBufferSize) \
		{ \
			Error("DataReader Buffer Overrun"); \
		} \
		T val = 0; \
		memcpy(&val, (reader->data) + (reader->offset), sizeof(T)); \
		(reader->offset) += sizeof(T); \
		return val; \
	}

DefineReadFunction(int8_t, ReadInt8);
DefineReadFunction(uint8_t, ReadUint8);

DefineReadFunction(int16_t, ReadInt16);
DefineReadFunction(uint16_t, ReadUint16);

DefineReadFunction(uint32_t, ReadUint32);
DefineReadFunction(int32_t, ReadInt32);

DefineReadFunction(uint64_t, ReadUint64);
DefineReadFunction(int64_t, ReadInt64);
DefineReadFunction(size_t, ReadSizeT);

DefineReadFunction(double, ReadDouble);
DefineReadFunction(float, ReadFloat);

void ReadBuffer(DataReader *reader, const size_t readSize, void *dest)
{
	if (reader->offset + readSize > reader->totalBufferSize)
	{
		Error("DataReader Buffer Overrun");
	}
	memcpy(dest, reader->data + reader->offset, readSize);
	reader->offset += readSize;
}

char *ReadStringSafe(DataReader *reader, size_t *outLength)
{
	size_t remainingSize = reader->totalBufferSize - reader->offset;
	if (remainingSize >= sizeof(size_t))
	{
		const size_t stringLength = ReadSizeT(reader);
		remainingSize -= sizeof(size_t);
		if (remainingSize >= sizeof(char) * stringLength)
		{
			char *string = calloc(stringLength, sizeof(char));
			CheckAlloc(string);
			memcpy(string, reader->data + reader->offset, stringLength);
			reader->offset += stringLength * sizeof(char);
			if (outLength)
			{
				*outLength = stringLength;
			}
			return string;
		}
	}
	return NULL;
}

void Seek(DataReader *reader, const size_t bytes)
{
	if (reader->offset + bytes > reader->totalBufferSize)
	{
		Error("DataReader Buffer Overrun");
	}
	reader->offset += bytes;
}

size_t DataReaderGetOffset(const DataReader *reader)
{
	return reader->offset;
}

uint16_t Checksum(const uint8_t *buffer, const size_t bufferSize)
{
	uint16_t checksum = 5873 + (bufferSize % 2367);
	for (size_t i = 0; i < bufferSize; i++)
	{
		checksum += buffer[i];
	}
	return checksum;
}
