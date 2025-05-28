//
// Created by droc101 on 5/28/25.
//

#include "KVList.h"
#include <string.h>
#include "Error.h"
#include "Logging.h"

void KvListCreate(KvList *list)
{
	if (!list)
	{
		return;
	}
	ListCreate(&list->keys);
	ListCreate(&list->values);
}

// TODO: This function should be optimized for faster lookups.
size_t KvIndexOf(const KvList *list, const char *key)
{
	for (size_t i = 0; i < list->keys.length; i++)
	{
		if (strcmp((const char *)ListGet(list->keys, i), key) == 0)
		{
			return i;
		}
	}
	return -1; // Not found
}

void KvSet(KvList *list, const char *key, const Param value)
{
	if (!list || !key)
	{
		return;
	}
	const size_t index = KvIndexOf(list, key);
	Param *p = (Param *)malloc(sizeof(Param));
	CheckAlloc(p);
	*p = value;
	char *k = strdup(key);
	if (index != -1)
	{
		ListSet(&list->values, index, p);
	} else
	{
		ListAdd(&list->keys, k);
		ListAdd(&list->values, p);
	}
}

Param *KvGet(const KvList *list, const char *key)
{
	if (!list || !key)
	{
		return NULL;
	}
	const size_t index = KvIndexOf(list, key);
	if (index != -1)
	{
		return (Param *)ListGet(list->values, index);
	}
	LogWarning("Tried to get key '%s' from KvList, but it does not exist.", key);
	return NULL; // Not found
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
		ListRemoveAt(&list->keys, index);
		ListRemoveAt(&list->values, index);
	} else
	{
		LogWarning("Tried to delete key '%s' from KvList, but it does not exist.", key);
	}
}

void KvListDestroy(KvList *list)
{
	if (!list)
	{
		LogWarning("Tried to destroy a NULL KvList.");
		return;
	}

	ListAndContentsFree(&list->keys, false);
	ListAndContentsFree(&list->values, false);
}

size_t KvListLength(const KvList *list)
{
	if (!list)
	{
		return 0;
	}
	return list->keys.length;
}

bool KvListHas(const KvList *list, const char *key)
{
	if (!list || !key)
	{
		return false;
	}
	const size_t index = KvIndexOf(list, key);
	return index != -1;
}

Param* KvGetTypeWithDefault(const KvList *list, const char *key, const ParamType expectedType, Param *defaultValue)
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
