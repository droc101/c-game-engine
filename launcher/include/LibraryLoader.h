//
// Created by droc101 on 11/9/2025.
//

#ifndef GAME_LIBRARYLOADER_H
#define GAME_LIBRARYLOADER_H

#ifdef WIN32
#include <minwindef.h>
typedef HMODULE LibraryHandle;
#else
typedef void* LibraryHandle;
#endif

#endif //GAME_LIBRARYLOADER_H

void LibraryLoaderSetup();

LibraryHandle OpenLibrary(const char *path);

void *OpenSymbol(LibraryHandle library, const char *symbol);

int CloseLibrary(LibraryHandle library);

const char *LibraryLoaderError();
