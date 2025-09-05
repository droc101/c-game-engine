//
// Created by droc101 on 4/21/2024.
//

#include "List.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <SDL_error.h>
#include <SDL_mutex.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Error.h"
#include "Logging.h"

void _ListInit(List *list, const enum _ListType listType)
{
	assert(list);
	assert(listType == LIST_POINTER || listType == LIST_UINT64 || listType == LIST_UINT32 || listType == LIST_INT32);

	list->length = 0;
	list->data = malloc(sizeof(struct _ListData));
	list->data->type = listType;
	list->data->pointerData = NULL;
}

void _LockingListInit(LockingList *list, const enum _ListType listType)
{
	assert(list);

	_ListInit((List *)list, listType);
	list->mutex = SDL_CreateMutex();
}


static inline void ListCopyHelper(const List *oldList, List *newList)
{
	assert(oldList->data);
	assert(newList->data);

	size_t listSize = 0;
	switch (oldList->data->type)
	{
		case LIST_POINTER:
		case LIST_UINT64:
			listSize = oldList->length * 8;
			break;
		case LIST_UINT32:
		case LIST_INT32:
			listSize = oldList->length * 4;
			break;
	}
	newList->data->pointerData = malloc(listSize);
	memcpy(newList->data->pointerData, oldList->data->pointerData, listSize);
	newList->length = oldList->length;
}

void _ListCopy(const List *oldList, List *newList)
{
	assert(oldList);
	assert(newList);

	_ListInit(newList, oldList->data->type);
	ListCopyHelper(oldList, newList);
}

void _LockingListCopy(const LockingList *oldList, LockingList *newList)
{
	assert(oldList);
	assert(newList);

	ListLock(*oldList);
	_LockingListInit(newList, oldList->data->type);
	ListCopyHelper((const List *)oldList, (List *)newList);
	ListUnlock(*oldList);
}


void _ListAdd(List *list, void *data)
{
	assert(list);

	switch (list->data->type)
	{
		case LIST_POINTER:
			list->data->pointerData = GameReallocArray(list->data->pointerData, list->length + 1, sizeof(void *));
			CheckAlloc(list->data->pointerData);
			list->data->pointerData[list->length] = data;
			break;
		case LIST_UINT64:
			list->data->uint64Data = GameReallocArray(list->data->uint64Data, list->length + 1, sizeof(uint64_t));
			CheckAlloc(list->data->uint64Data);
			list->data->uint64Data[list->length] = (uint64_t)(uintptr_t)data;
			break;
		case LIST_UINT32:
			list->data->uint32Data = GameReallocArray(list->data->uint32Data, list->length + 1, sizeof(uint32_t));
			CheckAlloc(list->data->uint32Data);
			list->data->uint32Data[list->length] = (uint32_t)(uintptr_t)data;
			break;
		case LIST_INT32:
			list->data->int32Data = GameReallocArray(list->data->int32Data, list->length + 1, sizeof(int32_t));
			CheckAlloc(list->data->int32Data);
			list->data->int32Data[list->length] = (int32_t)(uintptr_t)data;
			break;
	}
	list->length++;
}

void _LockingListAdd(LockingList *list, void *data)
{
	assert(list);

	ListLock(*list);
	_ListAdd((List *)list, data);
	ListUnlock(*list);
}


void _ListSet(const List *list, const size_t index, void *data)
{
	assert(list);
	assert(index <= list->length);

	switch (list->data->type)
	{
		case LIST_POINTER:
			list->data->pointerData[index] = data;
			break;
		case LIST_UINT64:
			list->data->uint64Data[index] = (uint64_t)(uintptr_t)data;
			break;
		case LIST_UINT32:
			list->data->uint32Data[index] = (uint32_t)(uintptr_t)data;
			break;
		case LIST_INT32:
			list->data->int32Data[index] = (int32_t)(uintptr_t)data;
			break;
	}
}

void _LockingListSet(const LockingList *list, const size_t index, void *data)
{
	assert(list);
	assert(index <= list->length);

	ListLock(*list);
	_ListSet((const List *)list, index, data);
	ListUnlock(*list);
}


void ListRemoveAtHelper(const List *list, const size_t index)
{
	switch (list->data->type)
	{
		case LIST_POINTER:
			memmove(&list->data->pointerData[index],
					&list->data->pointerData[index + 1],
					sizeof(void *) * (list->length - index));
			list->data->pointerData = GameReallocArray(list->data->pointerData, list->length, sizeof(void *));
			CheckAlloc(list->data->pointerData);
			break;
		case LIST_UINT64:
			memmove(&list->data->uint64Data[index],
					&list->data->uint64Data[index + 1],
					sizeof(uint64_t) * (list->length - index));
			list->data->uint64Data = GameReallocArray(list->data->uint64Data, list->length, sizeof(uint64_t));
			CheckAlloc(list->data->uint64Data);
			break;
		case LIST_UINT32:
			memmove(&list->data->uint32Data[index],
					&list->data->uint32Data[index + 1],
					sizeof(uint32_t) * (list->length - index));
			list->data->uint32Data = GameReallocArray(list->data->uint32Data, list->length, sizeof(uint32_t));
			CheckAlloc(list->data->uint32Data);
			break;
		case LIST_INT32:
			memmove(&list->data->int32Data[index],
					&list->data->int32Data[index + 1],
					sizeof(int32_t) * (list->length - index));
			list->data->int32Data = GameReallocArray(list->data->int32Data, list->length, sizeof(int32_t));
			CheckAlloc(list->data->int32Data);
			break;
	}
}

void _ListRemoveAt(List *list, const size_t index)
{
	assert(list);
	assert(index <= list->length);
	assert(list->length && list->data);

	list->length--;
	if (list->length == 0)
	{
		free(list->data->pointerData);
		list->data->pointerData = NULL;
		return;
	}
	ListRemoveAtHelper(list, index);
}

void _LockingListRemoveAt(LockingList *list, const size_t index)
{
	assert(list);
	assert(index <= list->length);
	assert(list->length && list->data);

	ListLock(*list);
	list->length--;
	if (list->length == 0)
	{
		free(list->data->pointerData);
		list->data->pointerData = NULL;

		ListUnlock(*list);
		return;
	}
	ListRemoveAtHelper((List *)list, index);
	ListUnlock(*list);
}


void _ListInsertAfter(List *list, size_t index, void *data)
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
	switch (list->data->type)
	{
		case LIST_POINTER:
			list->data->pointerData = GameReallocArray(list->data->pointerData, list->length, sizeof(void *));
			CheckAlloc(list->data->pointerData);
			memmove(&list->data->pointerData[index + 1],
					&list->data->pointerData[index],
					sizeof(void *) * (list->length - index - 1));
			list->data->pointerData[index] = data;
			break;
		case LIST_UINT64:
			list->data->uint64Data = GameReallocArray(list->data->uint64Data, list->length, sizeof(uint64_t));
			CheckAlloc(list->data->uint64Data);
			memmove(&list->data->uint64Data[index + 1],
					&list->data->uint64Data[index],
					sizeof(uint64_t) * (list->length - index - 1));
			list->data->uint64Data[index] = (uint64_t)(uintptr_t)data;
			break;
		case LIST_UINT32:
			list->data->uint32Data = GameReallocArray(list->data->uint32Data, list->length, sizeof(uint32_t));
			CheckAlloc(list->data->uint32Data);
			memmove(&list->data->uint32Data[index + 1],
					&list->data->uint32Data[index],
					sizeof(uint32_t) * (list->length - index - 1));
			list->data->uint32Data[index] = (uint32_t)(uintptr_t)data;
			break;
		case LIST_INT32:
			list->data->int32Data = GameReallocArray(list->data->int32Data, list->length, sizeof(int32_t));
			CheckAlloc(list->data->int32Data);
			memmove(&list->data->int32Data[index + 1],
					&list->data->int32Data[index],
					sizeof(int32_t) * (list->length - index - 1));
			list->data->int32Data[index] = (int32_t)(uintptr_t)data;
			break;
	}
}

void _LockingListInsertAfter(LockingList *list, const size_t index, void *data)
{
	assert(list);

	ListLock(*list);
	_ListInsertAfter((List *)list, index, data);
	ListUnlock(*list);
}


size_t _ListFind(const List *list, const void *data)
{
	if (!list->length || !list->data)
	{
		return -1;
	}

	for (size_t i = 0; i < list->length; i++)
	{
		switch (list->data->type)
		{
			case LIST_POINTER:
				if (list->data->pointerData[i] == data)
				{
					return i;
				}
				break;
			case LIST_UINT64:
				if (list->data->uint64Data[i] == (uint64_t)(uintptr_t)data)
				{
					return i;
				}
				break;
			case LIST_UINT32:
				if (list->data->uint32Data[i] == (uint32_t)(uintptr_t)data)
				{
					return i;
				}
				break;
			case LIST_INT32:
				if (list->data->int32Data[i] == (int32_t)(uintptr_t)data)
				{
					return i;
				}
				break;
		}
	}
	return -1;
}

size_t _LockingListFind(LockingList *list, const void *data)
{
	if (!list->length || !list->data)
	{
		return -1;
	}

	ListLock(*list);
	for (size_t i = 0; i < list->length; i++)
	{
		switch (list->data->type)
		{
			case LIST_POINTER:
				if (list->data->pointerData[i] == data)
				{
					ListUnlock(*list);
					return i;
				}
				break;
			case LIST_UINT64:
				if (list->data->uint64Data[i] == (uint64_t)(uintptr_t)data)
				{
					ListUnlock(*list);
					return i;
				}
				break;
			case LIST_UINT32:
				if (list->data->uint32Data[i] == (uint32_t)(uintptr_t)data)
				{
					ListUnlock(*list);
					return i;
				}
				break;
			case LIST_INT32:
				if (list->data->int32Data[i] == (int32_t)(uintptr_t)data)
				{
					ListUnlock(*list);
					return i;
				}
				break;
		}
	}
	ListUnlock(*list);
	return -1;
}


void _ListLock(const LockingList *list)
{
	assert(list);
	// TODO: Explodes on arm64 when the quit button is pressed
	if (SDL_LockMutex(list->mutex) < 0)
	{
		LogError("Failed to lock list mutex with error: %s\n", SDL_GetError());
		Error("Failed to lock list mutex!");
	}
}


void _ListUnlock(const LockingList *list)
{
	assert(list);
	if (SDL_UnlockMutex(list->mutex) < 0)
	{
		LogError("Failed to unlock list mutex with error: %s\n", SDL_GetError());
		Error("Failed to unlock list mutex!");
	}
}


void _ListClear(List *list)
{
	assert(list);

	list->length = 0;
	free(list->data->pointerData);
	list->data->pointerData = NULL;
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

	switch (list->data->type)
	{
		case LIST_POINTER:
		case LIST_UINT64:
			memset(list->data->pointerData, 0, list->length * 8);
			break;
		case LIST_UINT32:
		case LIST_INT32:
			memset(list->data->pointerData, 0, list->length * 4);
			break;
	}
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

	if (list->data)
	{
		free(list->data->pointerData);
		list->data->pointerData = NULL;
		free(list->data);
		list->data = NULL;
	}
	list->length = 0;
}

void _LockingListFree(LockingList *list)
{
	assert(list);

	ListLock(*list);
	_ListFree((List *)list);
	ListUnlock(*list);
	SDL_DestroyMutex(list->mutex);
	list->mutex = NULL;
}


void _ListFreeOnlyContents(const List *list)
{
	assert(list);
	assert(!list->data || list->data->type == LIST_POINTER);

	if (list->length && list->data)
	{
		for (size_t i = 0; i < list->length; i++)
		{
			free(list->data->pointerData[i]);
		}
	}
}

void _LockingListFreeOnlyContents(const LockingList *list)
{
	assert(list);
	assert(list->data->type == LIST_POINTER);

	ListLock(*list);
	if (list->length && list->data)
	{
		for (size_t i = 0; i < list->length; i++)
		{
			free(list->data->pointerData[i]);
		}
	}
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


void *GameReallocArray(void *ptr, const size_t arrayLength, const size_t elementSize)
{
	if (elementSize == 0)
	{
		LogWarning("GameReallocArray: elementSize is zero, returning NULL");
		return NULL;
	}
	if (arrayLength > SIZE_MAX / elementSize)
	{
		LogWarning("GameReallocArray: arrayLength * elementSize exceeds SIZE_MAX, returning NULL");
		errno = ENOMEM; // NOLINT(*-include-cleaner) (it's sadly not in a consistent file)
		return NULL;
	}
	return realloc(ptr, arrayLength * elementSize);
}
