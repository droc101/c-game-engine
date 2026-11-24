//
// Created by NBT22 on 9/2/25.
//

#include <assert.h>
#include <engine/helpers/AllocHelpers.h>
#include <engine/subsystem/Error.h>
#include <engine/subsystem/Logging.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void *AllocArray(const size_t arrayLength, const size_t elementSize)
{
	if (elementSize == 0)
	{
		LogWarning("AllocArray: elementSize is zero, returning NULL\n");
		return NULL;
	}
	if (arrayLength > SIZE_MAX / elementSize)
	{
		LogWarning("AllocArray: arrayLength * elementSize exceeds SIZE_MAX, returning NULL\n");
		errno = ENOMEM; // NOLINT(*-include-cleaner)
		return NULL;
	}
	return malloc(arrayLength * elementSize);
}

void *ReallocArray(void *ptr, const size_t arrayLength, const size_t elementSize)
{
	if (elementSize == 0)
	{
		LogWarning("ReallocArray: elementSize is zero, returning NULL\n");
		return NULL;
	}
	if (arrayLength > SIZE_MAX / elementSize)
	{
		LogWarning("ReallocArray: arrayLength * elementSize exceeds SIZE_MAX, returning NULL\n");
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

void *AvxAlignedCalloc(const size_t size)
{
#ifdef __AVX__
#ifdef WIN32
	void *data = _aligned_malloc(size, 32);
#else
	void *data = aligned_alloc(32, size);
#endif
	if (data)
	{
		return memset(data, 0, size);
	}
	return NULL;
#else
	return calloc(1, size);
#endif
}

void AvxAlignedFree(void *data)
{
	if (!data)
	{
		return;
	}
#if defined(__AVX__) && defined(WIN32)
	_aligned_free(data);
#else
	free(data);
#endif
}
