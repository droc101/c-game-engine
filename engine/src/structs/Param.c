//
// Created by droc101 on 11/16/25.
//

#include <engine/structs/Param.h>
#include <stddef.h>
#include <stdlib.h>

#include "engine/assets/DataReader.h"

void ReadParam(const void *data, const size_t dataSize, size_t *offset, Param *out)
{
	out->type = ReadByte(data, offset);
	switch (out->type)
	{
		case PARAM_TYPE_BYTE:
			out->byteValue = ReadByte(data, offset);
			break;
		case PARAM_TYPE_INTEGER:
			out->intValue = ReadInt(data, offset);
			break;
		case PARAM_TYPE_FLOAT:
			out->floatValue = ReadFloat(data, offset);
			break;
		case PARAM_TYPE_BOOL:
			out->boolValue = ReadByte(data, offset) != 0;
			break;
		case PARAM_TYPE_COLOR:
			out->colorValue.r = ReadFloat(data, offset);
			out->colorValue.g = ReadFloat(data, offset);
			out->colorValue.b = ReadFloat(data, offset);
			out->colorValue.a = ReadFloat(data, offset);
			break;
		case PARAM_TYPE_STRING:
			out->stringValue = ReadStringSafe(data, offset, dataSize, NULL);
			break;
		default:
			break;
	}
}

void FreeParam(Param *p)
{
	if (p->type == PARAM_TYPE_STRING)
	{
		free(p->stringValue);
	}
}
