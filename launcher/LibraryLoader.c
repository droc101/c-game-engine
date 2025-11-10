//
// Created by droc101 on 11/9/2025.
//

#include "LibraryLoader.h"

#ifdef WIN32
#include <windows.h>
#include <errhandlingapi.h>
#include <libloaderapi.h>
#include <minwindef.h>
#include <stddef.h>
#include <winbase.h>
#include <windef.h>
#include <winnt.h>

LibraryHandle OpenLibrary(const char *path)
{
	return LoadLibraryEx(path, NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
}

int CloseLibrary(const LibraryHandle library)
{
	return FreeLibrary(library);
}

void *OpenSymbol(const LibraryHandle library, const char *symbol)
{
	return GetProcAddress(library, symbol);
}

const char *LibraryLoaderError()
{
	const DWORD errCode = GetLastError();

	LPSTR msgBuf = NULL;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				   NULL,
				   errCode,
				   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				   (LPSTR)&msgBuf,
				   0,
				   NULL);

	return msgBuf;
}

#else
#include <dlfcn.h>

LibraryHandle OpenLibrary(const char *path)
{
	return dlopen(path, RTLD_NOW);
}

void *OpenSymbol(const LibraryHandle library, const char *symbol)
{
	return dlsym(library, symbol);
}

int CloseLibrary(const LibraryHandle library)
{
	return dlclose(library);
}

const char *LibraryLoaderError()
{
	return dlerror();
}

#endif
