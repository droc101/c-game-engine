//
// Created by droc101 on 4/27/2024.
//

#include "DataReader.h"
#include <string.h>

double ReadDouble(const byte *data, size_t *offset)
{
	double d;
	memcpy(&d, data + *offset, sizeof(double));
	*offset += sizeof(double);
	return d;
}

double ReadDoubleA(const byte *data, const size_t offset)
{
	double d;
	memcpy(&d, data + offset, sizeof(double));
	return d;
}

uint ReadUint(const byte *data, size_t *offset)
{
	uint i;
	memcpy(&i, data + *offset, sizeof(uint));
	*offset += sizeof(uint);
	return i;
}

int ReadInt(const byte *data, size_t *offset)
{
	int i;
	memcpy(&i, data + *offset, sizeof(int));
	*offset += sizeof(int);
	return i;
}

uint ReadUintA(const byte *data, const size_t offset)
{
	uint i;
	memcpy(&i, data + offset, sizeof(uint));
	return i;
}

float ReadFloat(const byte *data, size_t *offset)
{
	float f;
	memcpy(&f, data + *offset, sizeof(float));
	*offset += sizeof(float);
	return f;
}

byte ReadByte(const byte *data, size_t *offset)
{
	const byte b = data[*offset];
	*offset += sizeof(byte);
	return b;
}

void ReadString(const byte *data, size_t *offset, char *dest, const size_t len)
{
	strncpy(dest, (const char *)(data + *offset), len);
	*offset += len;
}

short ReadShort(const byte *data, size_t *offset)
{
	short s;
	memcpy(&s, data + *offset, sizeof(short));
	*offset += sizeof(short);
	return s;
}

void ReadBytes(const byte *data, size_t *offset, const size_t len, void *dest)
{
	memcpy(dest, data + *offset, len);
	*offset += len;
}

size_t ReadSizeT(const byte *data, size_t *offset)
{
	size_t i;
	memcpy(&i, data + *offset, sizeof(size_t));
	*offset += sizeof(size_t);
	return i;
}
