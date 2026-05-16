//
// Created by droc101 on 4/21/2024.
//

#include <assert.h>
#include <engine/helpers/Realloc.h>
#include <engine/structs/List.h>
#include <engine/subsystem/Error.h>
#include <limits.h>
#include <SDL3/SDL_mutex.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define __USE_GNU 1 // NOLINT(*-reserved-identifier)
#define __STDC_WANT_LIB_EXT1__ 1 // NOLINT(*-reserved-identifier)
#include <stdlib.h>

void _ListInit(List *list, const size_t stride)
{
	assert(list);

	list->data = NULL;
	list->length = 0;
	list->stride = stride;
}

void _LockingListInit(LockingList *list, const size_t stride)
{
	assert(list);

	_ListInit((List *)list, stride);
	list->mutex = SDL_CreateMutex();
}

void _SortedListInit(SortedList *list, const size_t stride, SortedListCompareFunction CompareFunction)
{
	assert(list);

	_ListInit((List *)list, stride);
	list->CompareFunction = CompareFunction;
}


static inline void ListCopyHelper(const List *oldList, List *newList)
{
	const size_t listSize = oldList->length * oldList->stride;
	assert(!newList->data);
	newList->data = malloc(listSize);
	CheckAlloc(newList->data);
	memcpy(newList->data, oldList->data, listSize);
	newList->length = oldList->length;
}

void _ListCopy(const List *restrict oldList, List *restrict newList)
{
	assert(oldList);
	assert(newList);

	_ListFree(newList);
	_ListInit(newList, oldList->stride);
	ListCopyHelper(oldList, newList);
}

void _LockingListCopy(const LockingList *restrict oldList, LockingList *restrict newList)
{
	assert(oldList);
	assert(newList);

	ListLock(*oldList);
	_LockingListFree(newList);
	_LockingListInit(newList, oldList->stride);
	ListCopyHelper((const List *)oldList, (List *)newList);
	ListUnlock(*oldList);
}


void _ListAdd(List *list, const void *data)
{
	assert(list);

	list->data = GameReallocArray(list->data, list->length + 1, list->stride);
	CheckAlloc(list->data);
	memcpy(list->data + (list->length * list->stride), data, list->stride);
	list->length++;
}

void _LockingListAdd(LockingList *list, const void *data)
{
	assert(list);

	ListLock(*list);
	_ListAdd((List *)list, data);
	ListUnlock(*list);
}

#ifdef WIN32
static int CompareFunction(void *function, const void *a, const void *b)
#else
static int CompareFunction(const void *a, const void *b, void *function)
#endif
{
	return ((SortedListCompareFunction)function)(*(const void **)a, *(const void **)b);
}

void _SortedListAdd(SortedList *list, const void *data)
{
	assert(list);

	// TODO: Fast insert logic
	_ListAdd((List *)list, data);
#ifdef WIN32
	qsort_s(list->data, list->length, list->stride, CompareFunction, list->CompareFunction);
#else
	qsort_r(list->data, list->length, list->stride, CompareFunction, list->CompareFunction);
#endif
}


void _ListSet(const List *list, const size_t index, const void *data)
{
	assert(list);
	assert(index <= list->length);

	memcpy(list->data + (index * list->stride), data, list->stride);
}

void _LockingListSet(const LockingList *list, const size_t index, const void *data)
{
	assert(list);
	assert(index <= list->length);

	ListLock(*list);
	_ListSet((const List *)list, index, data);
	ListUnlock(*list);
}


static inline void ListRemoveAtHelper(List *list, const size_t index)
{
	memmove(list->data + (index * list->stride),
			list->data + ((index + 1) * list->stride),
			list->stride * (list->length - index));
	list->data = GameReallocArray(list->data, list->length, list->stride);
	CheckAlloc(list->data);
}

void _ListRemoveAt(List *list, const size_t index)
{
	assert(list);
	assert(index <= list->length);
	assert(list->length);

	list->length--;
	if (list->length == 0)
	{
		free(list->data);
		list->data = NULL;
		return;
	}
	ListRemoveAtHelper(list, index);
}

void _LockingListRemoveAt(LockingList *list, const size_t index)
{
	assert(list);
	assert(index <= list->length);
	assert(list->length);

	ListLock(*list);
	list->length--;
	if (list->length == 0)
	{
		free(list->data);
		list->data = NULL;

		ListUnlock(*list);
		return;
	}
	ListRemoveAtHelper((List *)list, index);
	ListUnlock(*list);
}


void _ListInsertAfter(List *list, size_t index, const void *data)
{
	assert(list);

	if (list->length == 0 || index == SIZE_MAX)
	{
		_ListAdd(list, data);
		return;
	}

	assert(index <= list->length);

	index++;
	list->length++;
	list->data = GameReallocArray(list->data, list->length, list->stride);
	CheckAlloc(list->data);
	memmove(list->data + ((index + 1) * list->stride),
			list->data + (index * list->stride),
			list->stride * (list->length - index - 1));
	_ListSet(list, index, data);
}

void _LockingListInsertAfter(LockingList *list, const size_t index, const void *data)
{
	assert(list);

	ListLock(*list);
	_ListInsertAfter((List *)list, index, data);
	ListUnlock(*list);
}


size_t _ListFind(const List *list, const void *data)
{
	if (!list->length)
	{
		return SIZE_MAX;
	}

	for (size_t i = 0; i < list->length; i++)
	{
		if (memcmp(list->data + (i * list->stride), data, list->stride) == 0)
		{
			return i;
		}
	}
	return SIZE_MAX;
}

size_t _LockingListFind(LockingList *list, const void *data)
{
	if (!list->length)
	{
		return SIZE_MAX;
	}

	ListLock(*list);
	const size_t index = _ListFind((List *)list, data);
	ListUnlock(*list);
	return index;
}

size_t _SortedListFind(const SortedList *list, const void *data)
{
	assert(list);
	assert(list->CompareFunction);

	// Binary search
	size_t offset = 0;
	size_t length = list->length;
	while (length > 0)
	{
		const int val = list->CompareFunction(data, ListGet(*list, (length >> 1) + offset, const void *));
		if (val == 0)
		{
			return length >> 1;
		}
		if (val > 0)
		{
			offset++;
		}
		length >>= 1;
	}
	return SIZE_MAX;
}


void _ListLock(const LockingList *list)
{
	assert(list);
	// TODO: Explodes on arm64 when the quit button is pressed
	SDL_LockMutex(list->mutex); // SDL3: this function cannot fail
}


void _ListUnlock(const LockingList *list)
{
	assert(list);
	SDL_UnlockMutex(list->mutex); // SDL3: this function cannot fail
}


void _ListClear(List *list)
{
	assert(list);

	list->length = 0;
	free(list->data);
	list->data = NULL;
}

void _LockingListClear(LockingList *list)
{
	assert(list);

	ListLock(*list);
	_ListClear((List *)list);
	ListUnlock(*list);
}


void _ListZero(const List *list)
{
	assert(list);

	memset(list->data, 0, list->length * list->stride);
}

void _LockingListZero(const LockingList *list)
{
	assert(list);

	ListLock(*list);
	_ListZero((List *)list);
	ListUnlock(*list);
}


void _ListFree(List *list)
{
	assert(list);

	free(list->data);
	list->data = NULL;
	list->length = 0;
}

void _LockingListFree(LockingList *list)
{
	assert(list);

	ListLock(*list);
	_ListFree((List *)list);
	ListUnlock(*list);
	// TODO: There's a time of check/time of use issue here by unlocking the mutex before freeing it
	SDL_DestroyMutex(list->mutex);
	list->mutex = NULL;
}


void _ListFreeOnlyContents(const List *list)
{
	assert(list);

	if (list->length)
	{
		for (size_t i = 0; i < list->length; i++)
		{
			free(((void **)list->data)[i]);
		}
	}
}

void _LockingListFreeOnlyContents(const LockingList *list)
{
	assert(list);

	ListLock(*list);
	_ListFreeOnlyContents((List *)list);
	ListUnlock(*list);
}


void _ListAndContentsFree(List *list)
{
	assert(list);

	_ListFreeOnlyContents(list);
	_ListFree(list);
}

void _LockingListAndContentsFree(LockingList *list)
{
	assert(list);

	_LockingListFreeOnlyContents(list);
	_LockingListFree(list);
}
