//
// Created by droc101 on 5/28/25.
//

#include <engine/structs/KVList.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <engine/structs/Color.h>
#include <engine/structs/Param.h>

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
	assert(list && key && defaultValue);
	Param *p = KvGet(list, key);
	if (!p || p->type != expectedType)
	{
		return defaultValue;
	}
	return p;
}

#pragma endregion

void KvListCreate(KvList list)
{
	assert(list);
	KvList_init(list);
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

void KvSetUnsafe(KvList list, const char *key, const Param value)
{
	KvSet(list, key, value);
}

#pragma endregion
