//
// Created by droc101 on 11/9/2025.
//

#include "../include/LibraryLoader.h"

#ifdef WIN32
// clang-format off
#include <windows.h>
#include <errhandlingapi.h>
// clang-format on

#include <libloaderapi.h>
#include <minwindef.h>
#include <stddef.h>
#include <stdio.h>
#include <winbase.h>
#include <windef.h>
#include <winnt.h>
#else
#include <dlfcn.h>
#endif

void LibraryLoaderSetup()
{
#ifdef WIN32
	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	wchar_t fullPath[MAX_PATH];
	GetFullPathNameW(L"./bin/", MAX_PATH, fullPath, NULL);
	AddDllDirectory(fullPath);
#endif
	// No setup is needed on Linux
}

LibraryHandle OpenLibrary(const char *path)
{
#ifdef WIN32
	return LoadLibraryEx(path, NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
#else
	return dlopen(path, RTLD_NOW);
#endif
}

void *OpenSymbol(const LibraryHandle library, const char *symbol)
{
#ifdef WIN32
	return GetProcAddress(library, symbol);
#else
	return dlsym(library, symbol);
#endif
}

int CloseLibrary(const LibraryHandle library)
{
#ifdef WIN32
	return FreeLibrary(library);
#else
	return dlclose(library);
#endif
}

const char *LibraryLoaderError()
{
#ifdef WIN32
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
#else
	return dlerror();
#endif
}
