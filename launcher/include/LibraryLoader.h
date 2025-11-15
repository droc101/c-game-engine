//
// Created by droc101 on 11/9/2025.
//

#ifndef GAME_LIBRARYLOADER_H
#define GAME_LIBRARYLOADER_H

#define LIB_PREFIX "bin/"

#ifdef WIN32
#include <minwindef.h>
typedef HMODULE LibraryHandle;

#define LIB_SUFFIX ".dll"
#else
typedef void *LibraryHandle;

#define LIB_SUFFIX ".so"
#endif

#endif //GAME_LIBRARYLOADER_H

/**
 * Initialize the library loader
 */
void LibraryLoaderSetup();

/**
 * Open a shared library
 * @param path The path to the SO or DLL file
 * @return A handle to the library, or NULL on failure
 */
LibraryHandle OpenLibrary(const char *path);

/**
 * Get a symbol from a library
 * @param library The library to load the symbol from
 * @param symbol The symbol to load
 * @return The symbol pointer, or NULL on error
 */
void *OpenSymbol(LibraryHandle library, const char *symbol);

/**
 * Close a shared library
 * @param library The library to close
 * @return 0 on success
 */
int CloseLibrary(LibraryHandle library);

/**
 * Get the error from the library loader. This should be called immediately after a library loader function.
 * @return
 */
const char *LibraryLoaderError();
