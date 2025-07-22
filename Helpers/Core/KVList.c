//
// Created by droc101 on 5/28/25.
//

#include "KVList.h"
#include <assert.h>
#include <string.h>
#include "Error.h"
#include "Logging.h"

#pragma region Private Functions

/**
 * Get the index of a key in the key-value list.
 * @param list The list to search in.
 * @param key The key to search for.
 * @return The index of the key in the list, or -1 if not found.
 * @todo This should be optimized.
 */
size_t KvIndexOf(const KvList *list, const char *key)
{
	ListLock(list->keys);
	for (size_t i = 0; i < list->keys.length; i++)
	{
		if (strcmp((const char *)ListGetPointer(list->keys, i), key) == 0)
		{
			ListUnlock(list->keys);
			return i;
		}
	}
	ListUnlock(list->keys);
	return -1; // Not found
}

/**
 * Set a key-value pair in a kvlist
 * @param list The list to set the key-value pair in
 * @param key The key to set
 * @param value The value to set
 */
void KvSet(KvList *list, const char *key, const Param value)
{
	if (!list || !key)
	{
		return;
	}
	const size_t index = KvIndexOf(list, key);
	Param *p = malloc(sizeof(Param));
	CheckAlloc(p);
	*p = value;
	if (index != -1)
	{
		ListLock(list->values);
		free(ListGetPointer(list->values, index));
		ListSet(list->values, index, p);
		ListUnlock(list->values);
	} else
	{
		ListAdd(list->keys, strdup(key));
		ListAdd(list->values, p);
	}
}

/**
 * Get a value from the kvlist by key
 * @param list The list to get the value from
 * @param key The key to get the value for
 * @return The value associated with the key, or NULL if not found
 */
Param *KvGet(const KvList *list, const char *key)
{
	if (!list || !key)
	{
		return NULL;
	}
	const size_t index = KvIndexOf(list, key);
	if (index != -1)
	{
		return ListGetPointer(list->values, index);
	}
	LogWarning("Tried to get key '%s' from KvList, but it does not exist.\n", key);
	return NULL; // Not found
}

/**
 * Get a value from the kvlist by key, with a default value if the key does not exist or is of the wrong type
 * @param list The list to get the value from
 * @param key The key to get the value for
 * @param expectedType The expected type of the value
 * @param defaultValue The default value to return if the key does not exist or is of the wrong type
 * @return The value associated with the key, or the default value if not found or of the wrong type
 */
Param *KvGetTypeWithDefault(const KvList *list, const char *key, const ParamType expectedType, Param *defaultValue)
{
	if (!defaultValue)
	{
		Error("Passed NULL defaultValue to KvGetTypeWithDefault");
	}
	if (!list || !key)
	{
		return defaultValue;
	}
	Param *p = KvGet(list, key);
	if (!p || p->type != expectedType)
	{
		return defaultValue;
	}
	return p;
}

#pragma endregion

void KvListCreate(KvList *list)
{
	if (!list)
	{
		return;
	}
	ListInit(list->keys, LIST_POINTER);
	ListInit(list->values, LIST_POINTER);
}

void KvListDestroy(KvList *list)
{
	if (!list)
	{
		LogWarning("Tried to destroy a NULL KvList.");
		return;
	}

	ListAndContentsFree(list->keys);
	ListAndContentsFree(list->values);
}

void KvDelete(KvList *list, const char *key)
{
	if (!list || !key)
	{
		return;
	}
	const size_t index = KvIndexOf(list, key);
	if (index != -1)
	{
		ListRemoveAt(list->keys, index);
		ListRemoveAt(list->values, index);
	} else
	{
		LogWarning("Tried to delete key '%s' from KvList, but it does not exist.", key);
	}
}

size_t KvListLength(const KvList *list)
{
	if (!list)
	{
		return 0;
	}
	assert(list->keys.length == list->values.length);
	return list->keys.length;
}

bool KvListHas(const KvList *list, const char *key)
{
	if (!list || !key)
	{
		return false;
	}
	return KvIndexOf(list, key) != -1;
}

ParamType KvGetType(const KvList *list, const char *key)
{
	Param *p = KvGet(list, key);
	if (!p)
	{
		return PARAM_TYPE_NONE; // Key not found
	}
	return p->type;
}

#pragma region Public Getters

byte KvGetByte(const KvList *list, const char *key, const byte defaultValue)
{
	const Param *p = KvGetTypeWithDefault(list,
										  key,
										  PARAM_TYPE_BYTE,
										  (Param[]){{PARAM_TYPE_BYTE, .byteValue = defaultValue}});
	assert(p);
	return p->byteValue;
}

int KvGetInt(const KvList *list, const char *key, const int defaultValue)
{
	const Param *p = KvGetTypeWithDefault(list,
										  key,
										  PARAM_TYPE_INTEGER,
										  (Param[]){{PARAM_TYPE_INTEGER, .intValue = defaultValue}});
	assert(p);
	return p->intValue;
}

float KvGetFloat(const KvList *list, const char *key, const float defaultValue)
{
	const Param *p = KvGetTypeWithDefault(list,
										  key,
										  PARAM_TYPE_FLOAT,
										  (Param[]){{PARAM_TYPE_FLOAT, .floatValue = defaultValue}});
	assert(p);
	return p->floatValue;
}

bool KvGetBool(const KvList *list, const char *key, const bool defaultValue)
{
	const Param *p = KvGetTypeWithDefault(list,
										  key,
										  PARAM_TYPE_BOOL,
										  (Param[]){{PARAM_TYPE_BOOL, .boolValue = defaultValue}});
	assert(p);
	return p->boolValue;
}

const char *KvGetString(const KvList *list, const char *key, const char *defaultValue)
{
	const Param *p = KvGetTypeWithDefault(list,
										  key,
										  PARAM_TYPE_STRING,
										  (Param[]){{PARAM_TYPE_STRING, .stringValue = ""}});
	assert(p);
	return p->stringValue[0] ? p->stringValue : defaultValue;
}

Color KvGetColor(const KvList *list, const char *key, const Color defaultValue)
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

inline void KvSetByte(KvList *list, const char *key, const byte value)
{
	KvSet(list, key, (Param){PARAM_TYPE_BYTE, .byteValue = value});
}

inline void KvSetInt(KvList *list, const char *key, const int value)
{
	KvSet(list, key, (Param){PARAM_TYPE_INTEGER, .intValue = value});
}

inline void KvSetFloat(KvList *list, const char *key, const float value)
{
	KvSet(list, key, (Param){PARAM_TYPE_FLOAT, .floatValue = value});
}

inline void KvSetBool(KvList *list, const char *key, const bool value)
{
	KvSet(list, key, (Param){PARAM_TYPE_BOOL, .boolValue = value});
}

void KvSetString(KvList *list, const char *key, const char *value)
{
	if (!value)
	{
		value = "";
	}
	if (!list || !key)
	{
		return;
	}
	KvSet(list, key, (Param){PARAM_TYPE_STRING, .stringValue = ""});
	strncpy(KvGet(list, key)->stringValue, value, sizeof(KvGet(list, key)->stringValue) - 1);
}

inline void KvSetColor(KvList *list, const char *key, const Color value)
{
	KvSet(list, key, (Param){PARAM_TYPE_COLOR, .colorValue = value});
}

void KvSetUnsafe(KvList *list, const char *key, const Param value)
{
	KvSet(list, key, value);
}

#pragma endregion
