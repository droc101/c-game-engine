//
// Created by droc101 on 11/10/2024.
//

#ifndef PLATFORMHELPERS_H
#define PLATFORMHELPERS_H

#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Attempt to set Win32 DWM window attributes (dark mode, square corners)
 * @param window The window to set attributes for
 */
void SetDwmWindowAttribs(SDL_Window *window);

/**
 * Restart the game.
 */
_Noreturn void RestartProgram();

/**
 * Check if a path is absolute. This does not ensure the path exists.
 * @param path The path to check
 */
bool IsPathAbsolute(const char *path);

const char *GameStrCaseStr(const char *haystack, const char *needle);

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

void *AvxAlignedCalloc(size_t size);

void AvxAlignedFree(void *data);

#endif //PLATFORMHELPERS_H
