//
// Created by droc101 on 11/10/2024.
//

#ifndef PLATFORMHELPERS_H
#define PLATFORMHELPERS_H

#include <SDL_video.h>
#include <stdbool.h>

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

#endif //PLATFORMHELPERS_H
