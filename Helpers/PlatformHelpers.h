//
// Created by droc101 on 11/10/2024.
//

#ifndef PLATFORMHELPERS_H
#define PLATFORMHELPERS_H

#include <SDL_video.h>

/**
 * Attempt to set Win32 DWM window attributes (dark mode, square corners)
 * @param window The window to set attributes for
 */
void SetDwmWindowAttribs(SDL_Window *window);

#endif //PLATFORMHELPERS_H
