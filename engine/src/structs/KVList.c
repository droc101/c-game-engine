//
// Created by droc101 on 5/28/25.
//

#include <assert.h>
#include <engine/assets/DataReader.h>
#include <engine/structs/Color.h>
#include <engine/structs/KVList.h>
#include <engine/subsystem/Error.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#pragma region Param Functions

size_t ReadParam(const void *data, const size_t dataSize, size_t *offset, Param *out)
{
	const size_t initialOffset = *offset;
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
		case PARAM_TYPE_ARRAY:
			out->arrayValue.length = ReadSizeT(data, offset);
			out->arrayValue.data = malloc(sizeof(Param) * out->arrayValue.length);
			CheckAlloc(out->arrayValue.data);
			for (size_t i = 0; i < out->arrayValue.length; i++)
			{
				Param *arrayIndex = &out->arrayValue.data[i];
				(void)ReadParam(data, dataSize, offset, arrayIndex);
			}
			break;
		case PARAM_TYPE_KV_LIST:
			out->kvListValue = malloc(sizeof(KvList));
			(void)ReadKvList(data, dataSize, offset, out->kvListValue);
			break;
		case PARAM_TYPE_UINT_64:
			out->uint64value = ReadSizeT(data, offset);
			break;
		default:
			break;
	}
	return *offset - initialOffset;
}

void CopyParam(const Param *source, Param *dest)
{
	dest->type = source->type;
	if (source->type == PARAM_TYPE_STRING)
	{
		dest->stringValue = strdup(source->stringValue);
	} else if (source->type == PARAM_TYPE_ARRAY)
	{
		dest->arrayValue.length = source->arrayValue.length;
		dest->arrayValue.data = malloc(sizeof(Param) * source->arrayValue.length);
		for (size_t i = 0; i < source->arrayValue.length; i++)
		{
			// Can't simply memcpy because might not be POD
			CopyParam(&source->arrayValue.data[i], &dest->arrayValue.data[i]);
		}
	} else if (source->type == PARAM_TYPE_KV_LIST)
	{
		dest->kvListValue = malloc(sizeof(*dest->kvListValue));
		CheckAlloc(dest->kvListValue);
		KvListCreate(dest->kvListValue);
		KvListCopy(source->kvListValue, dest->kvListValue);
	} else
	{
		memcpy(dest, source, sizeof(Param)); // plain-old-data
	}
}

void FreeParam(Param *param)
{
	if (param->type == PARAM_TYPE_STRING)
	{
		free(param->stringValue);
		param->stringValue = NULL;
	} else if (param->type == PARAM_TYPE_ARRAY)
	{
		for (size_t i = 0; i < param->arrayValue.length; i++)
		{
			FreeParam(&param->arrayValue.data[i]);
		}
		free(param->arrayValue.data);
		param->arrayValue.data = NULL;
	} else if (param->type == PARAM_TYPE_KV_LIST)
	{
		KvListDestroy(param->kvListValue);
		free(param->kvListValue);
		param->kvListValue = NULL;
	}
}

#pragma endregion

#pragma region Private Functions

/**
 * Set a key-value pair in a kvlist
 * @param list The list to set the key-value pair in
 * @param key The key to set
 * @param value The value to set
 */
void KvSet(KvList list, const char *key, const Param value)
{
	assert(list && key);
	KvList_set_at(list, key, value);
}

/**
 * Get a value from the kvlist by key
 * @param list The list to get the value from
 * @param key The key to get the value for
 * @return The value associated with the key, or NULL if not found
 */
Param *KvGet(const KvList list, const char *key)
{
	assert(list && key);
	Param *p = KvList_get(list, key);
	return p;
}

/**
 * Get a value from the kvlist by key, with a default value if the key does not exist or is of the wrong type
 * @param list The list to get the value from
 * @param key The key to get the value for
 * @param expectedType The expected type of the value
 * @param defaultValue The default value to return if the key does not exist or is of the wrong type
 * @return The value associated with the key, or the default value if not found or of the wrong type
 */
Param *KvGetTypeWithDefault(const KvList list, const char *key, const ParamType expectedType, Param *defaultValue)
{
	assert(list && key);
	Param *param = KvGet(list, key);
	if (!param || param->type != expectedType)
	{
		return defaultValue;
	}
	return param;
}

#pragma endregion

void KvListCreate(KvList list)
{
	assert(list);
	KvList_init(list);
}

size_t ReadKvList(const void *data, const size_t dataSize, size_t *offset, KvList out)
{
	const size_t initialOffset = *offset;
	KvListCreate(out);
	const size_t numParams = ReadSizeT(data, offset);
	for (size_t _ = 0; _ < numParams; _++)
	{
		size_t keyLength = 0;
		char *key = ReadStringSafe(data, offset, dataSize, &keyLength);
		Param param;
		(void)ReadParam(data, dataSize, offset, &param);
		KvSetUnsafe(out, key, param);
		free(key);
		FreeParam(&param);
	}
	return *offset - initialOffset;
}

void KvListCopy(const KvList source, KvList dest)
{
	KvList_init_set(dest, source);
}

void KvListDestroy(KvList list)
{
	assert(list);
	KvList_clear(list);
}

void KvDelete(KvList list, const char *key)
{
	assert(list && key);
	KvList_erase(list, key);
}

bool KvHas(KvList list, const char *key, const ParamType expectedType)
{
	assert(list && key);
	Param *p = KvGet(list, key);
	if (!p || p->type != expectedType)
	{
		return false;
	}
	return true;
}

#pragma region Public Getters

uint8_t KvGetByte(const KvList list, const char *key, const uint8_t defaultValue)
{
	const Param *p = KvGetTypeWithDefault(list,
										  key,
										  PARAM_TYPE_BYTE,
										  (Param[]){{PARAM_TYPE_BYTE, .byteValue = defaultValue}});
	assert(p);
	return p->byteValue;
}

int KvGetInt(const KvList list, const char *key, const int defaultValue)
{
	const Param *p = KvGetTypeWithDefault(list,
										  key,
										  PARAM_TYPE_INTEGER,
										  (Param[]){{PARAM_TYPE_INTEGER, .intValue = defaultValue}});
	assert(p);
	return p->intValue;
}

float KvGetFloat(const KvList list, const char *key, const float defaultValue)
{
	const Param *p = KvGetTypeWithDefault(list,
										  key,
										  PARAM_TYPE_FLOAT,
										  (Param[]){{PARAM_TYPE_FLOAT, .floatValue = defaultValue}});
	assert(p);
	return p->floatValue;
}

bool KvGetBool(const KvList list, const char *key, const bool defaultValue)
{
	const Param *p = KvGetTypeWithDefault(list,
										  key,
										  PARAM_TYPE_BOOL,
										  (Param[]){{PARAM_TYPE_BOOL, .boolValue = defaultValue}});
	assert(p);
	return p->boolValue;
}

const char *KvGetString(const KvList list, const char *key, const char *defaultValue)
{
	const Param *p = KvGetTypeWithDefault(list,
										  key,
										  PARAM_TYPE_STRING,
										  (Param[]){{PARAM_TYPE_STRING, .stringValue = ""}});
	assert(p);
	return p->stringValue[0] ? p->stringValue : defaultValue;
}

Color KvGetColor(const KvList list, const char *key, const Color defaultValue)
{
	const Param *p = KvGetTypeWithDefault(list,
										  key,
										  PARAM_TYPE_COLOR,
										  (Param[]){{PARAM_TYPE_COLOR, .colorValue = defaultValue}});
	assert(p);
	return p->colorValue;
}

uint64_t KvGetUint64(const KvList list, const char *key, const uint64_t defaultValue)
{
	const Param *p = KvGetTypeWithDefault(list,
										  key,
										  PARAM_TYPE_UINT_64,
										  (Param[]){{PARAM_TYPE_UINT_64, .uint64value = defaultValue}});
	assert(p);
	return p->uint64value;
}

ParamArray *KvGetArray(const KvList list, const char *key)
{
	Param *p = KvGetTypeWithDefault(list, key, PARAM_TYPE_ARRAY, NULL);
	if (!p)
	{
		return NULL;
	}
	return &p->arrayValue;
}

#pragma endregion

#pragma region Public Setters

inline void KvSetByte(KvList list, const char *key, const uint8_t value)
{
	KvSet(list, key, (Param){PARAM_TYPE_BYTE, .byteValue = value});
}

inline void KvSetInt(KvList list, const char *key, const int value)
{
	KvSet(list, key, (Param){PARAM_TYPE_INTEGER, .intValue = value});
}

inline void KvSetFloat(KvList list, const char *key, const float value)
{
	KvSet(list, key, (Param){PARAM_TYPE_FLOAT, .floatValue = value});
}

inline void KvSetBool(KvList list, const char *key, const bool value)
{
	KvSet(list, key, (Param){PARAM_TYPE_BOOL, .boolValue = value});
}

void KvSetString(KvList list, const char *key, const char *value)
{
	assert(list && key);
	if (!value)
	{
		value = "";
	}
	KvSet(list, key, (Param){PARAM_TYPE_STRING, .stringValue = ""});
	strncpy(KvGet(list, key)->stringValue, value, sizeof(KvGet(list, key)->stringValue) - 1);
}

inline void KvSetColor(KvList list, const char *key, const Color value)
{
	KvSet(list, key, (Param){PARAM_TYPE_COLOR, .colorValue = value});
}

inline void KvSetUint64(KvList list, const char *key, const uint64_t value)
{
	KvSet(list, key, (Param){PARAM_TYPE_UINT_64, .uint64value = value});
}

void KvSetParamArray(KvList list, const char *key, const ParamArray array)
{
	KvSet(list, key, (Param){PARAM_TYPE_ARRAY, .arrayValue = array});
}

void KvSetUnsafe(KvList list, const char *key, const Param value)
{
	KvSet(list, key, value);
}

#pragma endregion
