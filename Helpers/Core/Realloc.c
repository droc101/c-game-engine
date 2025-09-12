//
// Created by NBT22 on 9/2/25.
//

#include "Realloc.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Error.h"
#include "Logging.h"

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
		errno = ENOMEM; // NOLINT(*-include-cleaner)
		return NULL;
	}
	return realloc(ptr, arrayLength * elementSize);
}

void *Recalloc(void *ptr, const size_t oldSize, const size_t newSize)
{
	assert(newSize >= oldSize);
	void *newAlloc = realloc(ptr, newSize);
	CheckAlloc(newAlloc);
	memset(newAlloc + oldSize, 0, newSize - oldSize);
	return newAlloc;
}
