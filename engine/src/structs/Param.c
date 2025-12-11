//
// Created by droc101 on 11/16/25.
//

#include <engine/assets/DataReader.h>
#include <engine/structs/Param.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

size_t ReadParam(const void *data, const size_t dataSize, size_t *offset, Param *out)
{
	out->type = ReadByte(data, offset);
	switch (out->type)
	{
		case PARAM_TYPE_BYTE:
			out->byteValue = ReadByte(data, offset);
			return 2;
			break;
		case PARAM_TYPE_INTEGER:
			out->intValue = ReadInt(data, offset);
			return 1 + sizeof(int32_t);
			break;
		case PARAM_TYPE_FLOAT:
			out->floatValue = ReadFloat(data, offset);
			return 1 + sizeof(float);
			break;
		case PARAM_TYPE_BOOL:
			out->boolValue = ReadByte(data, offset) != 0;
			return 2;
			break;
		case PARAM_TYPE_COLOR:
			out->colorValue.r = ReadFloat(data, offset);
			out->colorValue.g = ReadFloat(data, offset);
			out->colorValue.b = ReadFloat(data, offset);
			out->colorValue.a = ReadFloat(data, offset);
			return 1 + (sizeof(float) * 4);
			break;
		case PARAM_TYPE_STRING:
			size_t len = 0;
			out->stringValue = ReadStringSafe(data, offset, dataSize, &len);
			return 1 + sizeof(size_t) + len;
			break;
		default:
			break;
	}
	return 1;
}

void FreeParam(Param *param)
{
	if (param->type == PARAM_TYPE_STRING)
	{
		free(param->stringValue);
	}
}
