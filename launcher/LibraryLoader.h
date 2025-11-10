//
// Created by droc101 on 11/9/2025.
//

#ifndef GAME_LIBRARYLOADER_H
#define GAME_LIBRARYLOADER_H

#if WIN32
#include <minwindef.h>
typedef HMODULE LibraryHandle;
#else
typedef void* LibraryHandle;
#endif

#endif //GAME_LIBRARYLOADER_H

LibraryHandle OpenLibrary(const char *path);

void *OpenSymbol(LibraryHandle library, const char *symbol);

int CloseLibrary(LibraryHandle library);

const char *LibraryLoaderError();
