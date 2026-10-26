//
// Created by droc101 on 11/10/2024.
//

/**
 * @file PlatformHelpers.h
 * Utility functions for operations that need different implementations on Windows and Linux
 */

#ifndef PLATFORMHELPERS_H
#define PLATFORMHELPERS_H

#include <SDL3/SDL_mutex.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Restart the game.
 */
_Noreturn void RestartProgram();

/**
 * Check if a path is absolute. This does not ensure the path exists.
 * @param path The path to check
 */
bool IsPathAbsolute(const char *path);

/**
 * Case insensitive version of @c strstr
 * @param haystack The string to search in
 * @param needle The string to search for
 * @return The beginning of the substring, or NULL if there is no match
 */
const char *GameStrCaseStr(const char *haystack, const char *needle);

/**
 * Get the absolute/fully resolved file path, including link resolution
 * @param path The path to resolve
 * @return The resolved path, or NULL on error
 */
char *CanonicalFilePath(const char *path);

/**
 * Redirect FD originalFd to a new pipe, storing a copy of originalFd in originalFdCopy
 * @param originalFd The file descriptor to redirect
 * @param pipeFds Storage for file descriptors of new pipe
 * @param originalFdCopy Storage for the original file descriptor
 * @return Success/fail
 * @note The write-side of the pipe will be closed before this function returns
 * @private
 */
bool RedirectFd(int originalFd, int *pipeFds, int *originalFdCopy);

/**
 * Restore a redirected file descriptor
 * @param modifiedFd The file descriptor that was redirected to a pipe
 * @param pipeFds The pipe file descriptors
 * @param originalFd The original file descriptor before redirection
 * @note the read-side of the pipe will be closed by this function
 * @private
 */
void RestoreFd(int modifiedFd, int *pipeFds, int originalFd);

/**
 * Make a zero-initalized heap allocation aligned to AVX requirements
 * @param size The size of the allocation
 * @return Pointer to allocated memory, or @c NULL on failure
 */
void *AvxAlignedCalloc(size_t size);

/**
 * Free AVX-aligned memory (such as but not limited to memory allocated by @c AvxAlignedCalloc)
 * @param data The memory to free
 */
void AvxAlignedFree(void *data);

/**
 * Opens a file with the default program. Depends on xdg-open on Linux
 * @param filePath The file to open
 */
void OpenFileInDefaultProgram(const char *filePath);

/**
 * Converts a path to use forward slashes
 * @param path The path to fixup
 */
void FixupPath(char *path);

/**
 * Create a directory at the given path
 * @note 0750 permissions used on Linux
 * @param path The path of the directory to create
 * @return Success/failure
 */
bool MakeDirectory(const char *path);

/**
 * Raise the debugger (if one is listening)
 */
void RaiseDebugger();

/**
 * Print the stack trace of the current thread
 */
void PrintStackTrace();

/**
 * Fully thread-safe mutex destruction function, since SDL_DestroyMutex is not thread-safe
 * @param mutex A pointer to the mutex that should be destroyed
 */
void DestroyMutex(SDL_Mutex **mutex);

#endif //PLATFORMHELPERS_H
