//
// Created by droc101 on 11/10/2024.
//

#ifndef PLATFORMHELPERS_H
#define PLATFORMHELPERS_H

#include <SDL3/SDL_video.h>
#include <stdbool.h>

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
 */
bool RedirectFd(int originalFd, int *pipeFds, int *originalFdCopy);

/**
 * Restore a redirected file descriptor
 * @param modifiedFd The file descriptor that was redirected to a pipe
 * @param pipeFds The pipe file descriptors
 * @param originalFd The original file descriptor before redirection
 * @note the read-side of the pipe will be closed by this function
 */
void RestoreFd(int modifiedFd, int *pipeFds, int originalFd);

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
 * @note 0660 permissions used on Linux
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

#endif //PLATFORMHELPERS_H
