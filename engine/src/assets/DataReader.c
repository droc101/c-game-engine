//
// Created by droc101 on 4/27/2024.
//

#include <assert.h>
#include <engine/assets/DataReader.h>
#include <engine/subsystem/Error.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DefineReadFunction(T, name) \
	DeclareReadFunction(T, name) \
	{ \
		(void)totalBufferSize; \
		assert(*offset + sizeof(T) <= totalBufferSize); \
		T val = 0; \
		memcpy(&val, (data) + *(offset), sizeof(T)); \
		*(offset) += sizeof(T); \
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

void ReadBuffer(const uint8_t *data, size_t *offset, const size_t dataSize, const size_t readSize, void *dest)
{
	(void)dataSize;
	assert(*offset + readSize <= dataSize);
	memcpy(dest, data + *offset, readSize);
	*offset += readSize;
}

char *ReadStringSafe(const uint8_t *data, size_t *offset, const size_t totalBufferSize, size_t *outLength)
{
	size_t remainingSize = totalBufferSize - *offset;
	if (remainingSize >= sizeof(size_t))
	{
		const size_t stringLength = ReadSizeT(data, offset, totalBufferSize);
		remainingSize -= sizeof(size_t);
		if (remainingSize >= sizeof(char) * stringLength)
		{
			char *string = calloc(stringLength, sizeof(char));
			CheckAlloc(string);
			memcpy(string, data + *offset, stringLength);
			*offset += stringLength * sizeof(char);
			if (outLength)
			{
				*outLength = stringLength;
			}
			return string;
		}
	}
	return NULL;
}
