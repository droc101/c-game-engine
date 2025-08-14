//
// Created by droc101 on 4/27/2024.
//

#include "DataReader.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

double ReadDouble(const uint8_t *data, size_t *offset)
{
	double d = 0;
	memcpy(&d, data + *offset, sizeof(double));
	*offset += sizeof(double);
	return d;
}

double ReadDoubleA(const uint8_t *data, const size_t offset)
{
	double d = 0;
	memcpy(&d, data + offset, sizeof(double));
	return d;
}

uint32_t ReadUint(const uint8_t *data, size_t *offset)
{
	uint32_t i = 0;
	memcpy(&i, data + *offset, sizeof(uint32_t));
	*offset += sizeof(uint32_t);
	return i;
}

int ReadInt(const uint8_t *data, size_t *offset)
{
	int i = 0;
	memcpy(&i, data + *offset, sizeof(int));
	*offset += sizeof(int);
	return i;
}

uint32_t ReadUintA(const uint8_t *data, const size_t offset)
{
	uint32_t i = 0;
	memcpy(&i, data + offset, sizeof(uint32_t));
	return i;
}

float ReadFloat(const uint8_t *data, size_t *offset)
{
	float f = 0;
	memcpy(&f, data + *offset, sizeof(float));
	*offset += sizeof(float);
	return f;
}

uint8_t ReadByte(const uint8_t *data, size_t *offset)
{
	const uint8_t b = data[*offset];
	*offset += sizeof(uint8_t);
	return b;
}

void ReadString(const uint8_t *data, size_t *offset, char *dest, const size_t len)
{
	strncpy(dest, (const char *)(data + *offset), len);
	*offset += len;
}

short ReadShort(const uint8_t *data, size_t *offset)
{
	short s = 0;
	memcpy(&s, data + *offset, sizeof(short));
	*offset += sizeof(short);
	return s;
}

void ReadBytes(const uint8_t *data, size_t *offset, const size_t len, void *dest)
{
	memcpy(dest, data + *offset, len);
	*offset += len;
}

size_t ReadSizeT(const uint8_t *data, size_t *offset)
{
	size_t i = 0;
	memcpy(&i, data + *offset, sizeof(size_t));
	*offset += sizeof(size_t);
	return i;
}

char *ReadStringSafe(const uint8_t *data, size_t *offset, const size_t totalBufferSize, size_t *outLength)
{
	size_t remainingSize = totalBufferSize - *offset;
	if (remainingSize >= sizeof(size_t))
	{
		const size_t stringLength = ReadSizeT(data, offset);
		remainingSize -= sizeof(size_t);
		if (remainingSize >= sizeof(char) * stringLength)
		{
			char *string = calloc(stringLength, sizeof(char));
			memcpy(string, data + *offset, stringLength);
			*offset += stringLength * sizeof(char);
			*outLength = stringLength;
			return string;
		}
	}
	return NULL;
}
