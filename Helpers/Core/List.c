//
// Created by droc101 on 4/21/2024.
//

#include "List.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../defines.h"
#include "Error.h"
#include "Logging.h"

void ListCreate(List *list)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListCreate!");
	}

	list->length = 0;
	list->data = NULL;
	list->mutex = SDL_CreateMutex();
}

void ListAdd(List *list, void *data)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListAdd!");
	}

	ListLock(*list);

	list->data = GameReallocArray(list->data, list->length + 1, sizeof(void *));
	CheckAlloc(list->data);
	list->data[list->length] = data;
	list->length++;

	ListUnlock(*list);
}

void ListSet(List *list, const size_t index, void *data)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListSet!");
	}
	if (index >= list->length)
	{
		Error("Attempted to set an item past the end of a list!");
	}

	ListLock(*list);

	list->data[index] = data;

	ListUnlock(*list);
}

void ListAddBatched(List *list, const size_t count, ...)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListAddBatched!");
	}

	ListLock(*list);

	list->data = GameReallocArray(list->data, list->length + count, sizeof(void *));
	CheckAlloc(list->data);

	va_list args;
	va_start(args, count);
	for (size_t i = 0; i < count; i++)
	{
		list->data[list->length] = va_arg(args, void *);
		list->length++;
	}
	va_end(args);

	ListUnlock(*list);
}

void ListRemoveAt(List *list, const size_t index)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListRemoveAt!");
	}
	if (!list->length || !list->data)
	{
		Error("Attempted to shrink empty list!");
	}
	if (index >= list->length)
	{
		Error("Attempted to remove an item past the end of a list!");
	}

	ListLock(*list);

	list->length--;
	if (list->length == 0)
	{
		free(list->data);
		list->data = NULL;

		ListUnlock(*list);
		return;
	}
	memmove(&list->data[index], &list->data[index + 1], sizeof(void *) * (list->length - index));
	list->data = GameReallocArray(list->data, list->length, sizeof(void *));
	CheckAlloc(list->data);

	ListUnlock(*list);
}

void ListInsertAfter(List *list, size_t index, void *data)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListInsertAfter!");
	}
	if (index >= list->length)
	{
		Error("Attempted to insert an item past the end of a list!");
	}

	ListLock(*list);

	index++;
	list->length++;
	list->data = GameReallocArray(list->data, list->length, sizeof(void *));
	CheckAlloc(list->data);

	memmove(&list->data[index + 1], &list->data[index], sizeof(void *) * (list->length - index - 1));
	list->data[index] = data;

	ListUnlock(*list);
}

size_t ListFind(const List list, const void *data)
{
	if (!list.length || !list.data)
	{
		return -1;
	}

	ListLock(list);

	for (size_t i = 0; i < list.length; i++)
	{
		if (list.data[i] == data)
		{
			ListUnlock(list);
			return i;
		}
	}

	ListUnlock(list);
	return -1;
}

void ListLock(const List list)
{
	// TODO: Explodes on arm64 when the quit button is pressed
	if (SDL_LockMutex(list.mutex) < 0)
	{
		LogError("Failed to lock list mutex with error: %s\n", SDL_GetError());
		Error("Failed to lock list mutex!");
	}
}

void ListUnlock(const List list)
{
	if (SDL_UnlockMutex(list.mutex) < 0)
	{
		LogError("Failed to unlock list mutex with error: %s\n", SDL_GetError());
		Error("Failed to unlock list mutex!");
	}
}

void ListClear(List *list)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListClear!");
	}

	ListLock(*list);

	list->length = 0;
	free(list->data);
	list->data = NULL;

	ListUnlock(*list);
}

void ListFree(List *list, const bool freeListPointer)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListFree!");
	}

	ListLock(*list);
	free(list->data);
	list->data = NULL;
	list->length = 0;
	ListUnlock(*list);
	SDL_DestroyMutex(list->mutex);
	list->mutex = NULL;
	if (freeListPointer)
	{
		free(list);
	}
}

void ListFreeOnlyContents(const List list)
{
	ListLock(list);
	if (list.length && list.data)
	{
		for (size_t i = 0; i < list.length; i++)
		{
			free(list.data[i]);
		}
	}
	ListUnlock(list);
}

void ListAndContentsFree(List *list, const bool freeListPointer)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListAndContentsFree!");
	}

	ListFreeOnlyContents(*list);

	ListFree(list, freeListPointer);
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
		errno = ENOMEM;
		return NULL;
	}
	return realloc(ptr, arrayLength * elementSize);
}
