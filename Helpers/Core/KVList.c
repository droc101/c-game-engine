//
// Created by droc101 on 5/28/25.
//

#include "KVList.h"
#include <string.h>
#include "Error.h"
#include "Logging.h"

void KvListCreate(KvList *list)
{
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
